// Rtree.C
//
// Armand Zakarian
// May 1995
//
//
// Provides implementation of classes RtreeFile and RtreeFileScan
// R-tree access method for Minirel
//
//
#include <values.h>
#include <assert.h>

#include "db.h"
#include "buf.h"
#include "minirel.h"

// define RT_WITH_LOCKING to include locking calls implementing 2PL
#ifdef RT_WITH_LOCKING
#include "lock_manager.h"
extern page_def rlp ;		// rlp is used to set all locks
#endif


#include "RTPage.h"
#include "Rtree.h"

extern DB *db ;
extern BufMgr *bm ;
extern global_error minirel_error ;



// create a scan for a given RtreeFile with scan condition
// ``records whose covering rectangles intersect Box box''
RtreeScan::RtreeScan(RtreeFile *r, Box box)
{
  rtf = r ;
  sbox = box ;

  // pin root page
  Status s ;
  int root = ((RTPage *)(rtf->header_ptr))->page ;
  Page *p ;
  if((s=bm->pinPage(root, p)) != OK) {
    minirel_error.add_error(RTREE, BUFMGR) ;
    return ;
    // return RTREE ;
  }

#ifdef RT_WITH_LOCKING
  rlp.page_id = root ;
  if((s=lockMgr->lock_page(rlp, Shared)) !=OK) {
    return RTREE ;
  }
#endif

  // start a scan on root page and push it on the stack
  ps[h=0] = ((RTPage *)p)->new_scan() ;
}

// closes scan
RtreeScan::~RtreeScan() 
{
  // pop page scans from stack and unpin corresponding pages
  Status s ;
  for(int i=h ; i>=0 ; i--) {
    int page = ps[i]->rtpage->page ;
    delete ps[i] ;
    if((s=bm->unpinPage(page, 0)) != OK) {
      minirel_error.add_error(RTREE, BUFMGR) ;
      return ;
      // return RTREE ;
    }
  }
}

// set rid to next record satisfying scan condition
Status RtreeScan::get_next(RID &rid) 
{
  Status s ;
  int found ;
  while(1) {
    // stack is empty---no more records left
    if(h<0) return DONE ;
    char *b ;
    // scan page corresponding to top of stack until either
    // no more records or a record is found that satisfies RtreeFileScan cond
    while((found=ps[h]->next(b, rid)) && !rtf->intersect((Box)b, sbox)) ;
    // if such a record is found on a leaf page, we are done
    if(found && ps[h]->rtpage->is_leaf()) return OK ;
    if(found) {
      // not a leaf page---go down in R-tree
      // pin descendant, create page scan for it and push it on stack
      Page *p ;
      if((s=bm->pinPage(rid.pageNo, p)) != OK) {
        minirel_error.add_error(RTREE, BUFMGR) ;
        return RTREE ;
      }

#ifdef RT_WITH_LOCKING
      rlp.page_id = rid.pageNo ;
      if((s=lockMgr->lock_page(rlp, Shared)) !=OK) {
        return RTREE ;
      }
#endif

      ps[++h] = ((RTPage *)p)->new_scan() ;
    } else {
      // reached end of page scanned above---pop it from stack
      int page = ps[h]->rtpage->page ;
      delete ps[h--] ;
      if((s=bm->unpinPage(page, 0)) != OK) {
        minirel_error.add_error(RTREE, BUFMGR) ;
        return RTREE ;
      }
    }
  }
}

// delete current record;  adjust covering rectangles on path to root
// if last record on a page, deallocate page
Status RtreeScan::delete_current()
{
  // while page contains just one entry, deallocate it and go up in tree
  Status s ;
  while(h>0 && ps[h]->rtpage->slots_in_use()==1) {
    int page = ps[h]->rtpage->page ;
#ifdef RT_WITH_LOCKING
    rlp.page_id = page ;
    if((s=lockMgr->upgrade_lock(rlp, Exclusive)) !=OK) {
      return RTREE ;
    }
#endif
    delete ps[h--] ;
    if((s=bm->unpinPage(page, 0)) != OK) {
      minirel_error.add_error(RTREE, BUFMGR) ;
      return RTREE ;
    }
    if((s=bm->freePage(page)) != OK) {
      minirel_error.add_error(RTREE, BUFMGR) ;
      return RTREE ;
    }
  }

  // delete entry in first page with more than one entry
  Box bbox = rtf->new_box() ;
  if(h>=0) {
    int page = ps[h]->rtpage->page ;
#ifdef RT_WITH_LOCKING
    rlp.page_id = page ;
    if((s=lockMgr->upgrade_lock(rlp, Exclusive)) !=OK) {
      return RTREE ;
    }
#endif
    Page *t ;
    if((s=bm->pinPage(page, t)) != OK) {
      minirel_error.add_error(RTREE, BUFMGR) ;
      return RTREE ;
    }
    ps[h]->delete_current() ;
    if(h==0 && ps[h]->rtpage->slots_in_use()==0) {
      ps[h]->rtpage->set_pagetype(RTLeaf) ;
    } else {
      rtf->compute_bbox(bbox, ps[h]->rtpage) ;
    }
    if((s=bm->unpinPage(page, 1)) != OK) {
      minirel_error.add_error(RTREE, BUFMGR) ;
      return RTREE ;
    }
  }

  // adjust boxes on path up to the root
  int i = h ;
  int update = 1 ; // update is 1 if the descendent of this page was changed
  while(--i >= 0) {
    if(update && !rtf->same_box(bbox, (Box)(ps[i]->cur_key))) {
      int page = ps[i]->rtpage->page ;
      Page *t ;
      if((s=bm->pinPage(page, t)) != OK) {
        minirel_error.add_error(RTREE, BUFMGR) ;
        return RTREE ;
      }
#ifdef RT_WITH_LOCKING
      rlp.page_id = page ;
      if((s=lockMgr->upgrade_lock(rlp, Exclusive)) !=OK) {
        return RTREE ;
      }
#endif
      ps[i]->update_current((char *)bbox) ;
      rtf->compute_bbox(bbox, ps[i]->rtpage) ;
      if((s=bm->unpinPage(page, 1)) != OK) {
        minirel_error.add_error(RTREE, BUFMGR) ;
        return RTREE ;
      }
    } else {
      update = 0 ;
    }
  }
  delete[] bbox ;
  return OK ;
}

// if an index with a given name n exists, open it;
// else create an empty index
RtreeFile::RtreeFile(Status &status, const char *n, int d, int m) 
{
  strcpy(name, n) ; 
  dim = d ;
  // allowed range for dim is [1..6]
  if(dim<1 || dim>6) {
    minirel_error.add_error(RTREE, "Rtree: Bad dimension value") ;
    status = RTREE ;
    return ;
  }
  min_occ = m ;
  // compute max number of entries that fit on a page
  max_occ = (PAGESIZE-sizeof(lsn_t)-RTPAGEHEADER) / 
    (2*dim*sizeof(float)+sizeof(RID)+1) ;
  // project given value for min_occ on [1..max_occ/2]
  min_occ = max(min(min_occ, max_occ/2), 1) ;
  dirtyRoot = 0 ;
  destroyed = 1 ;
  status = OK ;
  int root ;

  Status s=db->get_file_entry(name, header) ;
  if(s==OK) {
    // index exists---pin header and root
    if((s=bm->pinPage(header, header_ptr)) != OK) {
      minirel_error.add_error(RTREE, BUFMGR) ;
      status = RTREE ;
      return ;
    }
#ifdef RT_WITH_LOCKING
    rlp.page_id = header ;
    if((s=lockMgr->lock_page(rlp, Shared)) !=OK) {
      status = RTREE ;
      return ;
    }
#endif
    // page field of header page points to root INSTEAD of header itself
    root = ((RTPage *)header_ptr)->page ;
    Page *r ;
    if((s=bm->pinPage(root, r)) != OK) {
      minirel_error.add_error(RTREE, BUFMGR) ;
      status = RTREE ;
      return ;
    }
    // read dim and max_occ from header page
    dim = ((RTPage *)header_ptr)->keysize / (2*sizeof(float)) ;
    max_occ = ((RTPage *)header_ptr)->nslots ;
    destroyed = 0 ;
  } else {
    // index doesn't exist; create it
    // first the header page
    if((s=bm->newPage(header, header_ptr)) != OK) {
      minirel_error.add_error(RTREE, BUFMGR) ;
      status = RTREE ;
      return ;
    } 
#ifdef RT_WITH_LOCKING
    rlp.page_id = header ;
    if((s=lockMgr->lock_page(rlp, Shared)) !=OK) {
      status = RTREE ;
      return ;
    }
#endif
    // allocate a root page and pin it
    Page *r ;
    if((s=bm->newPage(root, r)) != OK) {
      minirel_error.add_error(RTREE, BUFMGR) ;
      status = RTREE ;
      return ;
    } 
    // initialize header and root pages
    // NOTE page field of header page points to root
    ((RTPage *)header_ptr)->init(root, max_occ, 2*dim*sizeof(float)) ;
    ((RTPage *)r)->init(root,  max_occ, 2*dim*sizeof(float)) ;
    ((RTPage *)r)->set_pagetype(RTLeaf) ;
    dirtyRoot = 1 ;
    destroyed = 0 ;
    // add name of new index to catalog
    if((s=db->add_file_entry(name, header)) != OK) {
      minirel_error.add_error(RTREE, DBMGR) ;
      status = RTREE ;
      return ;
    } 
  }
}

// close index---basically unpin root and header
RtreeFile::~RtreeFile() 
{
  Status s ;
  int root = ((RTPage *)header_ptr)->page ;
  if(!destroyed) {
    if((s=bm->unpinPage(root, dirtyRoot)) != OK) {
      minirel_error.add_error(RTREE, BUFMGR) ;
    }
    if((s=bm->unpinPage(header, dirtyRoot)) != OK) {
      minirel_error.add_error(RTREE, BUFMGR) ;
    }
  }
}
// create a scan for this index 
IndexFileScan *RtreeFile::new_scan(const void *b)
{
  if(destroyed) {
    return 0 ;
  } else {
    return new RtreeScan(this, (Box)b) ;
  }
}

// delete a leaf entry with record_id recid given its box b
// simply scan index for Box b and delete the entry when found
Status RtreeFile::Delete(const void *b, const RID recid) 
{
  IndexFileScan *scan = new_scan(b) ;
  if(!scan) {
    minirel_error.add_error(RTREE, "Rtree: Delete error") ;
    return RTREE ;
  }
  RID rid ;
  while(scan->get_next(rid) == OK) 
    if(rid==recid) {
      Status s=scan->delete_current() ;
      delete scan ;
      return s ;
    }
  // reached end of scan without locating given recid
  delete scan ;
  return RECNOTFOUND ;
}

// destroy index by calling destroy_tree() on the root page
// and then drop index name from catalog
Status RtreeFile::destroy() 
{
  Status s ;
  if(destroyed) {
    minirel_error.add_error(RTREE, "Rtree: Index destroyed") ;
    return RTREE ;
  }

#ifdef RT_WITH_LOCKING
  rlp.page_id = header ;
  if((s=lockMgr->upgrade_lock(rlp, Exclusive)) !=OK) {
    return RTREE ;
  }
#endif

  int root = ((RTPage *)header_ptr)->page ;
  if((s=bm->unpinPage(root, 0)) != OK) {
    minirel_error.add_error(RTREE, BUFMGR) ;
    return RTREE ;
  }
  if((s=destroy_tree(root)) != OK) {
    minirel_error.add_error(RTREE, RTREE) ;
    return RTREE ;
  }
  // deallocate header page
  if((s=bm->unpinPage(header, 0)) != OK) {
    minirel_error.add_error(RTREE, BUFMGR) ;
    return RTREE ;
  }
  if((s=bm->freePage(header)) != OK) {
    minirel_error.add_error(RTREE, BUFMGR) ;
    return RTREE ;
  }
  if((s=db->delete_file_entry(name)) != OK) {
    minirel_error.add_error(RTREE, DBMGR) ;
    return RTREE ;
  }
  destroyed = 1 ;
  return OK ;
}


// recursively destroy (deallocate) tree rooted at page
Status RtreeFile::destroy_tree(int page)
{
  Status s ;
  Page *p ;
  if((s=bm->pinPage(page, p)) != OK) {
    minirel_error.add_error(RTREE, BUFMGR) ;
    return RTREE ;
  }
#ifdef RT_WITH_LOCKING
  rlp.page_id = page ;
  if((s=lockMgr->lock_page(rlp, Exclusive)) !=OK) {
    return RTREE ;
  }
#endif
  char *b ;
  RID rid ;
  // if not a leaf, invoke destroy_tree on children
  if(!((RTPage *)p)->is_leaf()) {
    RTPageScan *ps = ((RTPage *)p)->new_scan() ;
    while(ps->next(b, rid)) {
      if((s=destroy_tree(rid.pageNo)) != OK) {
        return RTREE ;
      }
    }
    delete ps ;
  }
  if((s=bm->unpinPage(page, 0)) != OK) {
    minirel_error.add_error(RTREE, BUFMGR) ;
    return RTREE ;
  }
  if((s=bm->freePage(page)) != OK) {
    minirel_error.add_error(RTREE, BUFMGR) ;
    return RTREE ;
  }
  return OK ;
}


int RtreeFile::intersect(Box box1, Box box2) 
{
  if(!box1 || !box2) return 1 ;

  for(int i=0 ; i<dim ; i++)
    if(box1[i][0]>box2[i][1] || box1[i][1]<box2[i][0]) return 0 ;
  return 1 ;
}


void RtreeFile::compute_bbox(Box bbox, RTPage *r) 
{
  for(int i=0 ; i<dim ; i++) { 
    bbox[i][0] = MAXFLOAT ;
    bbox[i][1] = -MAXFLOAT ;
  }
  RTPageScan *scan=r->new_scan() ;
  char *b ;
  RID rid ;
  while(scan->next(b, rid)) {
    add_to_box((Box)b, bbox) ;
  }
  delete scan ;
}
      
 
// insert entry <box, recid> in R-tree
Status RtreeFile::insert(const void *box, const RID recid) 
{
  if(destroyed) {
    minirel_error.add_error(RTREE, "Rtree: Index destroyed") ;
    return RTREE ;
  }

  Status s ;
  int root = ((RTPage *)header_ptr)->page ;
  // ps[] will contain RTPageScan* 's for page from root to parent of
  // leaf that is to receive record being inserted
  RTPageScan **ps = new RTPageScan *[RTMAXHEIGHT] ;
  assert(ps) ;
  int h=0 ;

  int page = root ;
  Page *p ; 
  // go from down in tree each time choosing descendent whose
  // covering rectangle needs to be least enlarged to cover new box
  while(1) {
    if((s=bm->pinPage(page, p)) != OK) {
      minirel_error.add_error(RTREE, BUFMGR) ;
      return RTREE ;
    }
#ifdef RT_WITH_LOCKING
    rlp.page_id = page ;
    if((s=lockMgr->lock_page(rlp, Shared)) !=OK) {
      return RTREE ;
    }
#endif
    if(((RTPage *)p)->is_leaf()) break ;
    RTPageScan *rs = ((RTPage *)p)->new_scan() ;
    char *b ;
    RID rid, min_rid ;
    double min_enl = MAXFLOAT ;
    // min_rid will be set to record_id pointing to descendent
    // whose rectangle needs least enlargement
    while(rs->next(b, rid)) {
      double enl = compute_enl((Box)box, (Box)b) ;
      if(enl < min_enl) {
        min_enl = enl ;
        min_rid = rid ;
      } 
    }
    delete rs ;
    rs = ((RTPage *)p)->new_scan() ;
    while(rs->next(b, rid)) {
      if(rid.pageNo == min_rid.pageNo) break ;
    }
    page = min_rid.pageNo ;
    ps[h++] = rs ;
  }

  Box box1 = new_box() ;
  Box box2 = new_box() ;
  RTPage *r ;
  RID newrid ; 
  int split ;

#ifdef RT_WITH_LOCKING
  rlp.page_id = page ;
  if((s=lockMgr->upgrade_lock(rlp, Exclusive)) !=OK) {
    return RTREE ;
  }
#endif

  // insert in leaf page splitting it if necessary
  if(((RTPage *)p)->slots_in_use() < max_occ) {
    // didn't need to split leaf
    ((RTPage *)p)->insert((char *)box, recid) ;
    split = 0 ;
  } else {
    // split leaf page
    if((s=split_page((RTPage *)p, (Box)box, recid, r)) != OK) {
      minirel_error.add_error(RTREE, RTREE) ;
      return RTREE ;
    }
    newrid.pageNo = r->page ;
    // compute covering rectangle for split image
    compute_bbox(box2, r) ;
    if((s=bm->unpinPage(r->page, 1)) != OK) {
      minirel_error.add_error(RTREE, BUFMGR) ;
      return RTREE ;
    }
    split = 1 ;
  }
  // compute covering rectangle for (updated) leaf page
  compute_bbox(box1, (RTPage *)p) ;
  if((s=bm->unpinPage(page, 1)) != OK) {
    minirel_error.add_error(RTREE, BUFMGR) ;
    return RTREE ;
  }

  int update = 1 ;
  // propagate splits and update boxes on path to root
  while(--h >= 0) {
    page = ps[h]->rtpage->page ;
    p = (Page *) ps[h]->rtpage ;
    // if the child(ren) were updated
    if(update) {
      // if page is not going to be split
      if(!split || ((RTPage *)p)->slots_in_use() < max_occ) {
        // update==1 if page is going to be updated
        update = split || !same_box((Box)ps[h]->cur_key, box1) ;
        if(update) {
#ifdef RT_WITH_LOCKING
          rlp.page_id = page ;
          if((s=lockMgr->upgrade_lock(rlp, Exclusive)) !=OK) {
            return RTREE ;
          }
#endif
          ps[h]->update_current((char *)box1) ;
          if(split) ((RTPage *)p)->insert((char*)box2, newrid) ;
        }
        split = 0 ;
      } else {
        // page is to be split
#ifdef RT_WITH_LOCKING
        rlp.page_id = page ;
        if((s=lockMgr->upgrade_lock(rlp, Exclusive)) !=OK) {
          return RTREE ;
        }
#endif
        if((s=split_page((RTPage *)p, box2, newrid, r)) != OK) {
          minirel_error.add_error(RTREE, RTREE) ;
          return RTREE ;
        }
        newrid.pageNo = r->page ;
        compute_bbox(box2, r) ;
        if((s=bm->unpinPage(r->page, 1)) != OK) {
          minirel_error.add_error(RTREE, BUFMGR) ;
          return RTREE ;
        }
        split = 1 ;
      }
      compute_bbox(box1, (RTPage *)p) ;
    }
    delete ps[h] ;
    if((s=bm->unpinPage(page, update)) != OK) {
      minirel_error.add_error(RTREE, BUFMGR) ;
      return RTREE ;
    }
  }

  if(split) {
    // new root
    RID newrid1 ;
    newrid1.pageNo = page ;
    Page *r ;
    if((s=bm->newPage(root, r)) != OK) {
      minirel_error.add_error(RTREE, BUFMGR) ;
      return RTREE ;
    } 
    // init new root and insert records for two children
    ((RTPage *)r)->init(root,  max_occ, 2*dim*sizeof(float)) ;
    ((RTPage *)r)->set_pagetype(RTInternal) ;
    ((RTPage *)r)->insert((char*)box1, newrid1) ;
    ((RTPage *)r)->insert((char*)box2, newrid ) ;
    dirtyRoot = 1 ;
#ifdef RT_WITH_LOCKING
    rlp.page_id = root ;
    if((s=lockMgr->lock_page(rlp, Exclusive)) !=OK) {
      return RTREE ;
    }
    rlp.page_id = header ;
    if((s=lockMgr->upgrade_lock(rlp, Exclusive)) !=OK) {
      return RTREE ;
    }
#endif
    // update header
    ((RTPage *)header_ptr)->init(root, max_occ, 2*dim*sizeof(float)) ;
  }
  delete[] ps ;
  delete[] box1 ;
  delete[] box2 ;

  return OK ;
}

double RtreeFile::compute_enl(Box box, Box b)
{
  double area_orig=1.0, area_new=1.0 ;
  for(int i=0 ; i<dim ; i++) {
    area_orig *= b[i][1]-b[i][0] ;
    area_new *= max(b[i][1], box[i][1])-min(b[i][0], box[i][0]) ;
  }
  return area_new-area_orig ;
}


// for an RTPage pointed to by p, set seed1 and seed2 
// so that 0<= seed1 < seed2 <=max_occ  and
// boxes of records on page numbered seed1 and seed2 are farthest apart 
// in space.  seed1==0 means that given Box box is the first seed
void RtreeFile::pick_seeds(RTPage *p, Box box, int &seed1, int &seed2)
{
  Box inbox = new_box() ;
  Box bbox = new_box() ; 
  int *ind_high = new int[dim] ;
  int *ind_low = new int[dim] ;

  for(int i=0 ; i<dim ; i++) ind_high[i] = ind_low[i] = 0 ; 
  copy_box(box, inbox) ;
  copy_box(box, bbox) ;

  RTPageScan *ps = p->new_scan() ;
  char *b ;
  RID rid ;
  int index=0 ;
  while(ps->next(b, rid)) {
    Box bb=(Box)b ;
    index++ ;
    for(int i=0 ; i<dim ; i++) {
      if(bb[i][1] <= inbox[i][1]) {
         inbox[i][1] = bb[i][1] ;
         ind_low[i] = index ;
      }
      if(bb[i][0] >= inbox[i][0]) {
         inbox[i][0] = bb[i][0] ;
         ind_high[i] = index ;
      }
    }
    add_to_box((Box)b, bbox) ;
  }
  delete ps ;

  double max_sep = -MAXFLOAT ;
  for(i=0 ; i<dim ; i++) {
    double sep = (inbox[i][0]-inbox[i][1])/(bbox[i][1]-bbox[i][0]) ;
    if(sep > max_sep) {
      max_sep = sep ;
      index = i ;
    }
  }
  seed1=min(ind_low[index], ind_high[index]) ; 
  seed2=max(ind_low[index], ind_high[index]) ; 
  if(seed1==seed2) {
    if(seed1) seed1-- ; 
    else seed2++ ;
  }

  delete[] inbox ;
  delete[] bbox ;
  delete[] ind_low ; delete[] ind_high ;
}

// split page pointed to by p, insert <box, recid> in either the
// original or the split image.  r is set to point to the split 
// image.  Splitting is according the Linear splitting algorithm.
Status RtreeFile::split_page(RTPage *p, Box box, RID recid, RTPage *&r) 
{
  Status s ;
  int page ;
  Page *rr ;
  // allocate and initialize split image
  if((s=bm->newPage(page, rr)) != OK) {
    minirel_error.add_error(RTREE, BUFMGR) ;
    return RTREE ;
  }
  r = (RTPage*)rr ;
  r->init(page, p->nslots, p->keysize) ;
  r->set_pagetype(p->pagetype) ;

#ifdef RT_WITH_LOCKING
  rlp.page_id = page ;
  if((s=lockMgr->lock_page(rlp, Exclusive)) !=OK) {
    return RTREE ;
  }
#endif

  // compute seeds for two clusters
  int seed1, seed2 ;
  pick_seeds(p, box, seed1, seed2) ;

  Box box1 = new_box() ;
  Box box2 = new_box() ;
  RTPageScan *ps = p->new_scan() ;
  char *b ;
  RID rid ;
  int index=0 ;
  // move first seed to split image
  while(ps->next(b, rid)) {
    index++ ;
    if(index==seed1) {
      copy_box((Box)b, box1) ;
      r->insert(b, rid) ;
      ps->delete_current() ;
    }
    if(index==seed2) {
      copy_box((Box)b, box2) ;
    }
  }
  delete ps ;
  if(seed1==0) {
    copy_box(box, box1) ;
    r->insert((char*)box, recid) ;
  } else {
    p->insert((char*)box, recid) ;
  }

  // num2 is number of entries already placed on original page,
  // num1 is number already placed on split image, num_left is number
  // undecided
  int num1=1, num2=1, num_left=max_occ-1 ;
  ps = p->new_scan() ;
  index = 0 ;
  // scan original page
  while(ps->next(b, rid)) {
    Box bb=(Box)b ;
    // if current is seed2, skip it
    if(++index == seed2) continue ;
    // if covering rectangle for cluster already placed on split image
    // needs to be enlarged less than covering rectangle of other cluster,
    // delete current from original page and put it on split image
    if(compute_enl(bb, box1)<compute_enl(bb, box2) && num2+num_left>min_occ ||
        num1+num_left==min_occ) {
      add_to_box(bb, box1) ; 
      r->insert(b, rid) ;
      ps->delete_current() ; 
      num1++ ;
    } else {
      add_to_box(bb, box2) ;
      num2++ ;
    }
    num_left-- ;
  }
  delete ps ;
  delete[] box1 ;
  delete[] box2 ;
  return OK ;
}



void RtreeFile::add_to_box(Box box, Box bbox) 
{
  for(int i=0 ; i<dim ; i++) {
    bbox[i][0] = min(bbox[i][0],  box[i][0]) ;        
    bbox[i][1] = max(bbox[i][1],  box[i][1]) ;        
  }
}

void RtreeFile::copy_box(Box s, Box d)
{
  for(int i=0 ; i<dim ; i++) {
    d[i][0] = s[i][0] ;
    d[i][1] = s[i][1] ;
  }
}

int RtreeFile::same_box(Box a, Box b)
{
  for(int i=0 ; i<dim ; i++) {
    if(a[i][0]!=b[i][0] || a[i][1]!=b[i][1]) return 0 ;
  }
  return 1 ;
}


Status RtreeFile::print_root()
{
  Status s ;
  int root = ((RTPage *)(header_ptr))->page ;
  Page *p ;
  if((s=bm->pinPage(root, p)) != OK) {
    minirel_error.add_error(RTREE, BUFMGR) ;
    return RTREE ;
  }
  ((RTPage *)p)->print() ;
  if((s=bm->unpinPage(root, 0)) != OK) {
    minirel_error.add_error(RTREE, BUFMGR) ;
    return RTREE ;
  }
}


