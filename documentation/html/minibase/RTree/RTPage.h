// RTPage.h
//
// Armand Zakarian
// May 1995
//
//
// Provides classes RTPage and RTPageScan 
// Used by the implementation of an R-tree access method for Minirel

#ifndef RTPAGE_H
#define RTPAGE_H



// define RT_WITH_RECOVERY to include logging of any operation
// that updates the page's contents
#ifdef RT_WITH_RECOVERY
#include "recovery_mgr.h"
extern RecoveryMgr *recoveryMgr ;
#endif

#include "page.h"
#include "minirel.h"


#define RTPAGEHEADER 	(2*sizeof(int)+2*sizeof(short))


class RTPageScan ;

// a page in the R-tree can be either internal or a leaf
enum RTPType {RTInternal=0x1, RTLeaf=0x2} ;

// RTPage is a general implementation of an (unclustered) index
// page containing keys of fixed length
// Records stored on the page have format <key, record_id>
class RTPage {
  friend RTPageScan ;
  
public:
  int page ;			// pageNo of this page
  RTPType pagetype ;		// type of page 
  short int nslots ;		// number of slots in the directory
  short int keysize ;		// length in bytes of key portion of rec

private:
  char ba[PAGESIZE-sizeof(lsn_t)-RTPAGEHEADER] ; 
                                // data area for storage of records
  char dir[0] ;			// directory start (grows backwards)
                                // directory is an array of bytes;
                                // dir[-(i+1)] is 0 if slot i is free, 1 o/w

public:

  RTPage() {} ; 		// RTPage 's will never be created
  ~RTPage() {} ; 		// but are rather used to impose structure
				// on a word-alligned area of memory

  // initializes the page
  // sets page, nslots, keysize
  // initializes the directory
  void init(int p, int n, int k) 
  { 
#ifdef RT_WITH_RECOVERY
    recoveryMgr->WriteUpdateLog(sizeof(int), page, 0, 
      (char*)this, (char*)&p, (Page*)this) ;
#endif
    page = p ;
    nslots = n ; 
    keysize = k ;
    for(char *p=dir ; p>dir-nslots ; *--p=0) ;
    
#ifdef RT_WITH_RECOVERY
    recoveryMgr->WriteUpdateLog(2*sizeof(int), page, sizeof(int), 
      (char*)this+sizeof(int), (char*)this+sizeof(int), (Page*)this) ;
    recoveryMgr->WriteUpdateLog(nslots, page, (char*)dir-(char*)this-nslots,
      (char*)dir-nslots, (char*)dir-nslots, (Page*)this) ;
#endif
    
  } ;

  // sets pagetype to t
  void set_pagetype(RTPType t) 
  { 
    
#ifdef RT_WITH_RECOVERY
    recoveryMgr->WriteUpdateLog(sizeof(RTPType), page, 
      (char*)&pagetype-(char*)this, (char*)&t, (char*)&pagetype, (Page*)this) ;
#endif
    
    pagetype = t ;
  } ;

  // returns 1 if pagetype is RTLeaf
  int is_leaf() const { return pagetype&RTLeaf ;} ;

  // returns number of occupied slots in the directory
  int slots_in_use() 
  { 
    int c=0 ;
    for(char *p=dir ; p>dir-nslots ; c+=*--p) ;
    return c ;
  } ;

  // finds a free slot and inserts <key, rid>
  // returns 0 if page is already full and 1 o/w
  int insert(const char *key, const RID &rid)
  {
    char *p = dir ;
    while(*--p && p>dir-nslots) ;
    if(*p) return 0 ;


    char one=1 ;
    
#ifdef RT_WITH_RECOVERY
    recoveryMgr->WriteUpdateLog(1, page, p-(char*)this, p, &one, (Page*)this) ;
#endif
    
    *p = 1 ;

    // starting address of record in the data area
    char *bb = ba+(dir-p-1)*(keysize+sizeof(RID)) ;
    
#ifdef RT_WITH_RECOVERY
    recoveryMgr->WriteUpdateLog(keysize, page, bb-(char*)this, 
      bb, key, (Page*)this) ;
#endif
    
    memcpy(bb, key, keysize) ;

    // starting address of record_id portion of record in the data area
    bb += keysize ;
    
#ifdef RT_WITH_RECOVERY
    recoveryMgr->WriteUpdateLog(sizeof(RID), page, bb-(char*)this, bb, 
      (char*)&rid, (Page*)this) ;
#endif
    
    memcpy(bb, &rid, sizeof(RID)) ;

    return 1 ;
  } ;

  // create a new scan for this page
  inline RTPageScan *new_scan() ;

  // pretty print all contents of the page ASSUMING key is a box (see Rtree.h)
  void print() ;
      
  
} ;

// defines a scan object for RTPage
class RTPageScan {
  friend RTPage ;

public:
  RTPage *rtpage ;	// the RTPage that is scanned
  char *cur_key ;	// pointer to current record in data area
private:
  char *current ;	// pointer to directory entry for current record

  RTPageScan(RTPage *r):  rtpage(r), current(r->dir) {} ;
  

public:
  ~RTPageScan() {} ;

  // sets key and rid to point to the key&record_id portions of the
  // next record
  // returns 0 if there is no next record
  int next(char *&key, RID &rid)
  {
    // find next occupied slot
    while(!*--current && current>rtpage->dir-rtpage->nslots) ;
    // if reached end of directory, return 0---no more records
    if(!*current) return 0 ;

    int i = rtpage->dir-current-1  ;
    cur_key = key = rtpage->ba+i*(rtpage->keysize+sizeof(RID)) ;
    rid = *(RID*)(key+rtpage->keysize) ;
    return 1 ;
  } ;
 
  // deletes the current record (sets directory entry to 0)
  void delete_current()
  {
    if(rtpage->dir-current > 0) {
      char zero=0 ;
      
#ifdef RT_WITH_RECOVERY
      recoveryMgr->WriteUpdateLog(1, rtpage->page, current-(char*)rtpage, 
        current, &zero, (Page*)rtpage) ;
#endif
      
      *current = 0 ;
    }
  } ;

  // updates the key portion of the current record with new data
  // pointed to by    key
  void update_current(const char *key)
  {
    int i = rtpage->dir-current-1 ;
    if(i>=0) {
      char *bb = rtpage->ba+i*(rtpage->keysize+sizeof(RID)) ;
      
#ifdef RT_WITH_RECOVERY
      recoveryMgr->WriteUpdateLog(rtpage->keysize, rtpage->page, 
        bb-(char*)rtpage, bb, key, (Page*)rtpage) ;
#endif
      
      memcpy(bb, key, rtpage->keysize) ;
    }
  } ;

} ;


inline RTPageScan *RTPage::new_scan() { return new RTPageScan(this) ;}

#endif

