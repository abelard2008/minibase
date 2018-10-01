/*
 * btfile.C - function members of class BTreeFile 
 * 
 * Johannes Gehrke & Gideon Glass  951022  CS564  UW-Madison
 */

#include "minirel.h"
#include "buf.h"
#include "db.h"
#include "new_error.h"
#include "btfile.h"
#include "btreefilescan.h"

const int MAGIC0 = 0xfeeb1e;

/* 
 * NOTE: (on error handling)  We use the `assert' macro to check the 
 * consistency of our internal data structures and variables, etc.  We
 * do not call MINIBASE_xxx_ERROR on these because failed asserts
 * would be due to bugs in our code and need to be flagged immediately (ie
 * runtime assert failed message) instead of propagating upwards as normal
 * errors do.  Failed asserts are not due to normal causes of db failure.
 */


const char* BTreeFile::Errors[BTreeFile::NR_ERRORS] = {
  "No error---this is for `OK'",	// _OK
  "db can't find header page",  // CANT_FIND_HEADER
  "buffer manager failed to pin header page", //CANT_PIN_HEADER,
  "buffer manager failed to allocate block for header page",//CANT_ALLOC_HEADER
  "couldn't register new index file w/ db", // CANT_ADD_FILE_ENTRY
  "can't unpin header page",		    // CANT_UNPIN_HEADER
  "can't pin index/leaf page",		    // CANT_PIN_PAGE
  "can't unpin index/leaf page",	    // CANT_UNPIN_PAGE
  "attempt to use invalid scan object",	    // INVALID_SCAN
  "SortedPage failed to delete current rid", // DELETE_CURRENT_FAILED
  "db failed to delete file entry",	     // CANT_DELETE_FILE_ENTRY
  "buffer manager failed to free a page",    // CANT_FREE_PAGE,
  "_destroyFile failed on a subtree",	     // CANT_DELETE_SUBTREE,
  "BTreeFile::insert : key too long",        // KEY_TOO_LONG
  "BtreeFile::insert : insert failed",       // INSERT_FAILED
  "BTreeFile::insert : could not create new root", // COULD_NOT_CREATE_ROOT
  "could not delete a data entry",		   // DELETE_DATAENTRY_FAILED
  "could not find data entry to delete",	   // DATA_ENTRY_NOT_FOUND
  "get_page_no on BTIndexPage failed",       // CANT_GET_PAGE_NO
  "bm::newPage failed",                      // CANT_ALLOCATE_NEW_PAGE
  "could not split leaf page",         // CANT_SPLIT_LEAF_PAGE
  "could not split index page",         // CANT_SPLIT_INDEX_PAGE
  

};


static error_string_table btree_table( BTREE, BTreeFile::Errors );
static error_string_table btlp_table( BTLEAFPAGE, BTLeafPage::Errors );
static error_string_table btip_table( BTINDEXPAGE, BTIndexPage::Errors );
static error_string_table sp_table( SORTEDPAGE, SortedPage::Errors );


/*
 *  BTreeFile::BTreeFile (Status& returnStatus, const char *filename)
 *
 *  an index with given filename should already exist,
 *  this opens it.
 */

BTreeFile::BTreeFile (Status& returnStatus, const std::string filename)
{
  Status st;

  st = MINIBASE_DB->get_file_entry(filename, headerPageId);
  CHECK_RAISE_ERROR_CTOR(st, returnStatus, BTREE, CANT_FIND_HEADER);

  st = MINIBASE_BM->pinPage(headerPageId, (Page *&) headerPage);
  CHECK_RAISE_ERROR_CTOR(st, returnStatus, BTREE, CANT_PIN_HEADER);

  dbname = strdup(filename.c_str());

  assert(headerPage->magic0 == MAGIC0);

  // ASSERTIONS:
  /*
   *
   * - headerPageId is the PageId of this BTreeFile's header page;
   * - headerPage, headerPageId valid and pinned
   * - dbname contains a copy of the name of the database
   */

  returnStatus = OK;
}

/*
 * BTreeFile::BTreeFile (Status& returnStatus, const char *filename, 
 *		      const AttrType keytype, const int keysize)
 * 
 * Open B+ tree index, creating w/ specified keytype and size if necessary.
 */

BTreeFile::BTreeFile (Status& returnStatus, const std::string filename, 
		      const AttrType keytype, const int keysize, int delete_fashion)
{
  Status st;  
 
  st = MINIBASE_DB->get_file_entry(filename, headerPageId);
  if (st != OK)
  {
    // create new BTreeFile; first, get a header page.
    st = MINIBASE_BM->newPage(headerPageId, (Page *&) headerPage);
    if (st != OK) {
      headerPageId = INVALID_PAGE;
      headerPage = NULL;
      RAISE_ERROR_CTOR(returnStatus, BTREE, CANT_ALLOC_HEADER);
    }
    
    // now add btreefile to the naming service of the database
    st = MINIBASE_DB->add_file_entry(filename, headerPageId);
    if (st != OK) {
      headerPageId = INVALID_PAGE;
      headerPage = NULL;
      MINIBASE_BM->freePage(headerPageId);
      RAISE_ERROR_CTOR(returnStatus, BTREE, CANT_ADD_FILE_ENTRY);
    }

    // initialize headerpage; we actually never reference prevPage or nextPage,
    // though.


    ((HFPage*) headerPage)->init(headerPageId);
    ((HFPage*) headerPage)->setNextPage(INVALID_PAGE);
    ((HFPage*) headerPage)->setPrevPage(INVALID_PAGE);
    headerPage->magic0 = MAGIC0;
    headerPage->root = INVALID_PAGE;
    headerPage->key_type = keytype;    
    headerPage->keysize = keysize;
    headerPage->delete_fashion = delete_fashion;


  }
  else {
    // open an existing btreefile

    st = MINIBASE_BM->pinPage(headerPageId, (Page *&) headerPage);
    CHECK_RAISE_ERROR_CTOR(st, returnStatus, BTREE, CANT_PIN_HEADER);
    assert(headerPage->magic0 == MAGIC0);
  }

  dbname = strdup(filename.c_str());


  // ASSERTIONS:
  /*
   * - headerPageId is the PageId of this BTreeFile's header page;
   * - headerPage points to the pinned header page (headerPageId)
   * - dbname contains the name of the database
   */

  returnStatus = OK;

#ifdef BT_TRACE
  if ( Trace )
    {
      *Trace << "STARTUP " << keysize << " " << delete_fashion << std::endl;
      Trace->flush();
    }
#endif

}

/*
 * BTreeFile::~BTreeFile ()
 *
 * minor cleanup work.  Unpin headerPageId if necessary.
 * (It may have been blown away by a destroyFile() previously.)
 */

BTreeFile::~BTreeFile ()
{
  free(dbname);

  if (headerPageId != INVALID_PAGE)
  {
    Status st=MINIBASE_BM->unpinPage(headerPageId);
    CHECK_ADD_ERROR(st, BTREE, CANT_UNPIN_PAGE);
  }

}


/*
 * Status BTreeFile::updateHeader (PageId newRoot)
 *
 * Change root of B+ tree to specified new root. 
 * 
 * Modifies the header page and feeds the dirty bit to buffer manager.
 */

Status BTreeFile::updateHeader (PageId newRoot)
{
  Status st;
  BTreeHeaderPage *pheader;
   PageId old_data;

  st = MINIBASE_BM->pinPage(headerPageId, (Page *&) pheader);
  CHECK_RAISE_ERROR(st, BTREE, CANT_PIN_HEADER);
 
  old_data = pheader->root;
  pheader->root = newRoot;
  

  // clock in dirty bit to bm so our dtor needn't have to worry about it
  st = MINIBASE_BM->unpinPage(headerPageId, 1 /* = DIRTY */ );
  CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_HEADER);

  // ASSERTIONS:
  // - headerPage, headerPageId valid, pinned and marked as dirty

  return OK;
}

/*
 *  Status BTreeFile::destroyFile ()
 *
 * Destroy entire index file.
 * 
 * Most work done recursively by _destroyFile(). 
 */

Status BTreeFile::destroyFile ()
{
  Status st;

  if (headerPage->root != INVALID_PAGE) {
    // if tree non-empty
    st = _destroyFile(headerPage->root);
    if (st != OK) return st; // if it encountered an error, it would've added it
  }
  
  st = MINIBASE_BM->unpinPage(headerPageId);
  CHECK_ADD_ERROR(st, BTREE, CANT_UNPIN_PAGE);

  PageId hdrId = headerPageId;	// Deal with the possibility 
				// that the freePage might fail.
  headerPageId = INVALID_PAGE;
  headerPage   = NULL;

  st = MINIBASE_BM->freePage(hdrId);
  CHECK_ADD_ERROR(st, BTREE, CANT_FREE_PAGE);

  st = MINIBASE_DB->delete_file_entry(dbname);
  CHECK_ADD_ERROR(st, BTREE, CANT_DELETE_FILE_ENTRY);

  // the destructor will take care of freeing the memory at dbname
  
  return OK;
}

/*
 * Status BTreeFile::_destroyFile (PageId pageno) 
 *
 * Recursively free all nodes of a B+ tree starting at specified
 * page, which is freed last of all.
 */
 
Status BTreeFile::_destroyFile (PageId pageno) 
{
  Status st;
  SortedPage *pagep;

  st = MINIBASE_BM->pinPage(pageno, (Page *&) pagep);
  CHECK_RAISE_ERROR(st, BTREE, CANT_PIN_PAGE);


  nodetype ndtype = (nodetype) pagep->get_type();  
  if (ndtype == INDEX) {
    RID		 rid;
    PageId       childId;
    BTIndexPage* ipagep = (BTIndexPage *) pagep;

    for (st = ipagep->get_first(rid, NULL, childId);
	 st != NOMORERECS;
         st = ipagep->get_next(rid, NULL, childId)) {

      Status tmpst = _destroyFile(childId);
      
      if (tmpst != OK) {
	ADD_ERROR(BTREE, CANT_DELETE_SUBTREE);
      }
    }
  }
  else {
    assert(ndtype == LEAF);
  }

  // ASSERTIONS:
  // - if pagetype == INDEX: the subtree rooted at pageno is completely
  //                         destroyed

  st = MINIBASE_BM->unpinPage(pageno);
  CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
  st = MINIBASE_BM->freePage(pageno);
  CHECK_RAISE_ERROR(st, BTREE, CANT_FREE_PAGE);

  // ASSERTIONS:
  // - pageno invalid and set free

  return OK;
}

/*
 * Status BTreeFile::insert (const void *key, const RID rid)
 *
 * insert recid with the key.  
 *
 * (`recid' is an opaque RID specified by the user; we don't look
 * at its contents at all.)
 *
 * Most work done recursively by _insert, which propogates up a new
 * root if the old root happened to split.
 *
 * Special case: create root if it previously didn't exist (i.e., the
 * index had no entries).
 */

Status BTreeFile::insert (const void *key, const RID rid)
{
  Status returnStatus;
  KeyDataEntry  newRootEntry;
  int           newRootEntrySize;
  KeyDataEntry* newRootEntryPtr = &newRootEntry;

  if (get_key_length(key, headerPage->key_type) > headerPage->keysize)
    RAISE_ERROR(BTREE, KEY_TOO_LONG);

  // TWO CASES:
  // 1. headerPage->root == INVALID_PAGE:
  //    - the tree is empty and we have to create a new first page;
  //      this page will be a leaf page
  // 2. headerPage->root != INVALID_PAGE:
  //    - we call _insert() to insert the pair (key, rid)

#ifdef BT_TRACE
  if ( Trace )
    {
      *Trace << "INSERT " << rid.pageNo << " " << rid.slotNo << " " << (char*)key << std::endl;
      *Trace << "DO" << std::endl;
      Trace->flush();
    }
#endif
  
  if (headerPage->root == INVALID_PAGE) {
    Status st;
    PageId newRootPageId;
    BTLeafPage *newRootPage;
    RID dummyrid;

    st = MINIBASE_BM->newPage(newRootPageId, (Page *&) newRootPage);
    CHECK_RAISE_ERROR(st, BTREE, COULD_NOT_CREATE_ROOT);

#ifdef BT_TRACE
  if ( Trace )
    {
      *Trace << "NEWROOT " << newRootPageId << std::endl;
      Trace->flush();
    }
#endif


    newRootPage->init(newRootPageId);
    newRootPage->setNextPage(INVALID_PAGE);
    newRootPage->setPrevPage(INVALID_PAGE);
    newRootPage->set_type((char) LEAF);

    // ASSERTIONS:
    // - newRootPage, newRootPageId valid and pinned
  
    st = newRootPage->insertRec(key,
				headerPage->key_type,
				rid, dummyrid);
    CHECK_ADD_ERROR(st, BTREE, COULD_NOT_CREATE_ROOT);
    CHECK_ADD_ERROR(st, BTREE, INSERT_FAILED);

#ifdef BT_TRACE
  if ( Trace )
    {
      *Trace << "PUTIN node " << newRootPageId << std::endl;
      Trace->flush();
    }
#endif
    
    st = MINIBASE_BM->unpinPage(newRootPageId, TRUE /* = DIRTY */);
    CHECK_ADD_ERROR(st, BTREE, CANT_UNPIN_PAGE);
    CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);

    updateHeader(newRootPageId);

#ifdef BT_TRACE
  if ( Trace )
    {
      *Trace << "DONE\n";
      Trace->flush();
    }
#endif
    
    return OK;
  }

  // ASSERTIONS:
  // - headerPageId, headerPage valid and pinned
  // - headerPage->root holds the pageId of the root of the B-tree
  // - none of the pages of the tree is pinned yet

#ifdef BT_TRACE
  if ( Trace )
    {
      *Trace << "SEARCH\n";
      Trace->flush();
    }
#endif
  
  returnStatus = _insert(key, rid, &newRootEntryPtr,
			 &newRootEntrySize, headerPage->root);
  CHECK_RAISE_ERROR(returnStatus, BTREE, INSERT_FAILED);

  // TWO CASES:
  // - newRootEntryPtr != NULL: a leaf split propagated up to the root
  //				and the root split: the new pageNo is in
  //                            newChildEntry->data->pageNo 
  // - newRootEntryPtr == NULL: no new root was created;
  //                            information on headerpage is still valid

  // ASSERTIONS:
  // - no page pinned

  if (newRootEntryPtr != NULL)
  {
    BTIndexPage* newRootPage;
    PageId       newRootPageId;
    Keytype      newEntryKey;
    PageId       newEntryPageId;
    RID          newEntryReturnRid;
    Status       st;
	     // the information about the pair <key, PageId> is
	     // packed in **goingUp: extract it
    get_key_data((void*)&newEntryKey, (Datatype *)&newEntryPageId,
		 &newRootEntry, newRootEntrySize, INDEX);
    assert(newEntryPageId != INVALID_PAGE);
	     // we have to allocate a new INDEX page and
	     // to redistribute the index entries
    st = MINIBASE_BM->newPage(newRootPageId, (Page *&) newRootPage);
    CHECK_ADD_ERROR(st, BTREE, COULD_NOT_CREATE_ROOT);
    CHECK_RAISE_ERROR(st, BTREE, INSERT_FAILED);

    assert(newRootPage != NULL && newRootPageId != INVALID_PAGE);
    newRootPage->init(newRootPageId);
    newRootPage->set_type((char) INDEX);

    // ASSERTIONS:
    // - newRootPage, newRootPageId valid and pinned
    // - newEntryKey, newEntryPage contain the data for the new entry
    //     which was given up from the level down in the recursion

#ifdef BT_TRACE
  if ( Trace )
    {
      *Trace << "NEWROOT " << newRootPageId << std::endl;
      Trace->flush();
    }
#endif

    st = newRootPage->insertKey((void*)&newEntryKey, 
				headerPage->key_type,
				newEntryPageId, newEntryReturnRid);
    CHECK_ADD_ERROR(st, BTREE, COULD_NOT_CREATE_ROOT);
    CHECK_RAISE_ERROR(st, BTREE, INSERT_FAILED);

    // the old root split and is now the left child of the new root
    newRootPage->setPrevPage(headerPage->root);

    st = MINIBASE_BM->unpinPage(newRootPageId, TRUE /* = DIRTY */);
    CHECK_ADD_ERROR(st, BTREE, CANT_UNPIN_PAGE);

    updateHeader(newRootPageId);

  }

#ifdef BT_TRACE
  if ( Trace )
    {
      *Trace << "DONE\n";
      Trace->flush();
    }
#endif
  
  return OK;
}

/*
 * 
 * Status BTreeFile::_insert (const void    *key,
 * 			   const RID     rid,
 * 			   KeyDataEntry  **goingUp,
 * 			   int           *goingUpSize,
 * 			   PageId        currentPageId)
 *
 * Do a recursive B+ tree insert of data entry <key, rid> into tree rooted 
 * at page currentPageId.
 *
 * If this page splits, copy (if we're on a leaf) or push (if on an index page)
 * middle entry up by setting *goingUp to it.  Otherwise (no split) set
 * *goingUp to NULL.
 * 
 * Code is long, but fairly straighforward.  Two big cases for INDEX and LEAF
 * pages.  (We use a switch for clarity, not because we expect more 
 * page types to appear.) 
 */

Status BTreeFile::_insert (const void    *key,
			   const RID     rid,
			   KeyDataEntry  **goingUp,
			   int           *goingUpSize,
			   PageId        currentPageId)
			  
{
  Status st;
  SortedPage*  rpPtr;

  assert(currentPageId != INVALID_PAGE);
  assert(*goingUp != NULL);
  
  st = MINIBASE_BM->pinPage(currentPageId,(Page *&) rpPtr);
  CHECK_RAISE_ERROR(st, BTREE, CANT_PIN_PAGE);


  nodetype pageType = (nodetype)rpPtr->get_type();

#ifdef BT_TRACE
  if ( Trace )
    {
      *Trace << "VISIT node " << currentPageId << std::endl;
      Trace->flush();
    }
#endif
 
  // TWO CASES:
  // - pageType == INDEX:
  //   recurse and then split if necessary
  // - pageType == LEAF:
  //   try to insert pair (key, rid), maybe split

  switch(pageType)
  {
  case INDEX:
  {
    BTIndexPage* currentIndexPage=(BTIndexPage *)rpPtr;
    PageId       currentIndexPageId = currentPageId;
    PageId nextPageId;

    st = currentIndexPage->get_page_no(key, headerPage->key_type, nextPageId);
    assert(nextPageId != INVALID_PAGE);
    CHECK_RAISE_ERROR(st, BTREE, CANT_GET_PAGE_NO);

    // now unpin the page, recurse and then pin it again

    st = MINIBASE_BM->unpinPage(currentIndexPageId);
    CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);

    st = _insert(key, rid, goingUp, goingUpSize, nextPageId);
    if (st != OK) // error will be handled on higher level
      return st;

    // two cases:
    // - *goingUp == NULL: one level lower no split has occurred:
    //                     we are done.
    // - *goingUp != NULL: one of the children has split and
    //                     **goingUp is the new data entry which has
    //                    to be inserted on this index page

    if (*goingUp == NULL)
      return OK;

    st = MINIBASE_BM->pinPage(currentPageId, (Page*&) currentIndexPage);


    CHECK_ADD_ERROR(st, BTREE, CANT_PIN_PAGE);
    if (st != OK)
    {
      MINIBASE_BM->unpinPage(currentPageId);
      return st;
    }

    // ASSERTIONS:
    // - *goingUp != NULL
    // - currentIndexPage, currentIndexPageId valid and pinned

    Keytype newEntryKey;
    PageId  newEntryPageId;
    RID     newEntryReturnRid;
    
		     // the information about the pair <key, PageId> is
		     // packed in **goingUp: extract it
    get_key_data((void*)&newEntryKey, (Datatype *)&newEntryPageId,
		 *goingUp, *goingUpSize, INDEX);
    assert(newEntryPageId != INVALID_PAGE);
      
    // check whether there can still be entries inserted on that page
    if (currentIndexPage->available_space() >=
	get_key_data_length(key, headerPage->key_type, INDEX))
    {
      *goingUp = NULL;		     // no split has occurred

      st = currentIndexPage->insertKey((void*)&newEntryKey, 
				       headerPage->key_type,
				       newEntryPageId, newEntryReturnRid);
      CHECK_ADD_ERROR(st, BTREE, INSERT_FAILED);
      st = MINIBASE_BM->unpinPage(currentIndexPageId, TRUE /* DIRTY */);
      CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
      
      return st;
    }

    // ASSERTIONS:
    // - on the current index page is not enough space available -> it splits
    //   therefore we have to allocate a new index page and we will
    //   distribute the entries
    // - currentIndexPage, currentIndexPageId valid and pinned

    BTIndexPage* newIndexPage;
    PageId       newIndexPageId;
		     // we have to allocate a new INDEX page and
		     // to redistribute the index entries
    st = MINIBASE_BM->newPage(newIndexPageId, (Page *&) newIndexPage);
    CHECK_RAISE_ERROR(st, BTREE, CANT_ALLOCATE_NEW_PAGE);


    newIndexPage->init(newIndexPageId);
    newIndexPage->set_type((char) INDEX);

#ifdef BT_TRACE
  if ( Trace )
    {
      if (headerPage->root != currentIndexPageId) 
          *Trace << "SPLIT node " << currentIndexPageId << " IN nodes "
                 << currentIndexPageId << " " << newIndexPageId << std::endl;
      else
          *Trace << "ROOTSPLIT IN nodes " << currentIndexPageId << " " << newIndexPageId << std::endl;
      Trace->flush();
    }
#endif
    
    // ASSERTIONS:
    // - newIndexPage, newIndexPageId valid and pinned
    // - currentIndexPage, currentIndexPageId valid and pinned
    // - newEntryKey, newEntryPage contain the data for the new entry which was
    //     given up from the level down in the recursion

    Datatype     tmpDatatype;
    Keytype      tmpKey;
    PageId       tmpPageId;
    RID insertRid;
    RID delRid;

    for (st = currentIndexPage->get_first(delRid, &tmpKey, tmpPageId);
	 st != NOMORERECS;
	 st = currentIndexPage->get_first(delRid, &tmpKey, tmpPageId))
    {
      st = newIndexPage->insertKey(&tmpKey, headerPage->key_type,
				   tmpPageId, insertRid);
      if (st != OK) // error will be handled in higher level
	return st;

      st = currentIndexPage->deleteRecord(delRid);
      if (st != OK) // error will be handled in higher level
	return st;
    }

    assert(currentIndexPage->empty());
    assert(!newIndexPage->empty());

    // ASSERTIONS:
    // - currentIndexPage empty
    // - newIndexPage holds all former records from currentIndexPage


    // we will try to make an equal split
    RID firstRid;
    for (st = newIndexPage->get_first(firstRid, (void*)&tmpKey, tmpPageId);
	 (currentIndexPage->available_space() >
	  newIndexPage->available_space());
	 st = newIndexPage->get_first(firstRid, (void*)&tmpKey, tmpPageId))
    {
		     // get the record from the current index page
      if (st != OK) // error will be handled in higher level
      {
	MINIBASE_BM->unpinPage(currentIndexPageId);
	MINIBASE_BM->unpinPage(newIndexPageId);
	return st;
      }

		     // now insert the <key,pageId> pair on the new
		     // index page
      st = currentIndexPage->insertKey(&tmpKey, headerPage->key_type,
				   tmpPageId, insertRid);
      if (st != OK) // error will be handled in higher level
      {
	MINIBASE_BM->unpinPage(currentIndexPageId);
	MINIBASE_BM->unpinPage(newIndexPageId);
	return st;
      }

      st = newIndexPage->deleteRecord(firstRid);
      if (st != OK) // error will be handled in higher level
      {
	MINIBASE_BM->unpinPage(currentIndexPageId);
	MINIBASE_BM->unpinPage(newIndexPageId);
	return st;
      }
    }


     // check whether <newKey, newIndexPageId>
     // will be inserted
     // on the newly allocated or on the old index page

    st = newIndexPage->get_first(firstRid, (void*)&tmpKey, tmpPageId);
    CHECK_RAISE_ERROR(st, BTREE, INSERT_FAILED);

    if (keyCompare((void*)&newEntryKey,(void*)&tmpKey,
		   headerPage->key_type) >= 0)
    {
      // the new data entry belongs on the new index page
      st = newIndexPage->insertKey((void*)&newEntryKey, headerPage->key_type,
				   newEntryPageId, insertRid);
      CHECK_RAISE_ERROR(st, BTREE, INSERT_FAILED);

      
    }
    else {
      st = currentIndexPage->insertKey((void*)&newEntryKey, 
				       headerPage->key_type,
				       newEntryPageId, insertRid);
      CHECK_RAISE_ERROR(st, BTREE, INSERT_FAILED);

    }

    st = MINIBASE_BM->unpinPage(currentIndexPageId, TRUE /* dirty */);
    CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
        
    // fill *goingUp
    st = newIndexPage->get_first(delRid, (void*)&tmpKey, tmpPageId);
    CHECK_RAISE_ERROR(st, BTREE, INSERT_FAILED);

    // now set prevPageId of the newIndexPage to the pageId
    // of the deleted entry:
    newIndexPage->setPrevPage(tmpPageId);

    tmpDatatype.pageNo = newIndexPageId;
    make_entry(*goingUp, headerPage->key_type, (void*)&tmpKey,
	       INDEX, tmpDatatype, goingUpSize);

    // delete first record on new index page since it is given up
    st = newIndexPage->deleteRecord(delRid);
    if (st != OK) // error will be handled in higher level
      return st;

    st = MINIBASE_BM->unpinPage(newIndexPageId, TRUE /* dirty */);
    CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);

#ifdef BT_TRACE
	if ( Trace ){
    	trace_children(currentIndexPageId);
    	trace_children(newIndexPageId);
	}
#endif
    
    // ASSERTIONS:
    // - no pages pinned
    // - *goingUp holds the pointer to the KeyDataEntry which is
    //   to be inserted on the index page one level up
    // - *goingUpSize holds size of *goingUp (set by make_entry)
    break;
  }

  case LEAF:
  {
    BTLeafPage*  currentLeafPage= (BTLeafPage *)rpPtr;
    PageId       currentLeafPageId = currentPageId;

    assert(*goingUp != NULL);

    // ASSERTIONS:
    // - *goingUp != NULL
    // - currentLeafPage, currentLeafPageId valid and pinned

    RID     newEntryReturnRid;

    // check whether there can still be entries inserted on that page
    if (currentLeafPage->available_space() >=
	get_key_data_length(key, headerPage->key_type, LEAF))
    {
      *goingUp = NULL;		     // no split has occurred
      *goingUpSize = -1;

      st = currentLeafPage->insertRec(key, 
				      headerPage->key_type,
				      rid, newEntryReturnRid);
      CHECK_ADD_ERROR(st, BTREE, INSERT_FAILED);
      st = MINIBASE_BM->unpinPage(currentLeafPageId /* DIRTY */);
      CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);

#ifdef BT_TRACE
  if ( Trace )
    {
      *Trace << "PUTIN node " << currentLeafPageId << std::endl;
      Trace->flush();
    }
#endif
      
      return OK;
    }

    // ASSERTIONS:
    // - on the current leaf page is not enough space available -> it splits
    // - therefore we have to allocate a new leaf page and we will
    // - distribute the entries

    BTLeafPage*  newLeafPage;
    PageId       newLeafPageId;
		     // we have to allocate a new LEAF page and
		     // to redistribute the data entries entries
    st = MINIBASE_BM->newPage(newLeafPageId, (Page *&) newLeafPage);
    CHECK_RAISE_ERROR(st, BTREE, CANT_ALLOCATE_NEW_PAGE);


    newLeafPage->init(newLeafPageId);
    newLeafPage->set_type((char) LEAF);
    newLeafPage->setNextPage(currentLeafPage->getNextPage());
    newLeafPage->setPrevPage(currentLeafPageId);  // for dbl-linked list
    currentLeafPage->setNextPage(newLeafPageId);

    // change the prevPage pointer on the next page:

    PageId rightPageId;
    rightPageId = newLeafPage->getNextPage();
    if (rightPageId != INVALID_PAGE)
    {
      SortedPage* rightPage;
      st = MINIBASE_BM->pinPage(rightPageId, (Page *&) rightPage);
      CHECK_ADD_ERROR(st, BTREE, CANT_PIN_PAGE);

      assert(rightPage != NULL);


      rightPage->setPrevPage(newLeafPageId);
      st = MINIBASE_BM->unpinPage(rightPageId, TRUE /* = DIRTY */);
      CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
    }


    // ASSERTIONS:
    // - newLeafPage, newLeafPageId valid and pinned
    // - currentLeafPage, currentLeafPageId valid and pinned

#ifdef BT_TRACE
  if ( Trace )
    {
      if (headerPage->root != currentLeafPageId) 
          *Trace << "SPLIT node " << currentLeafPageId << " IN nodes "
                 << currentLeafPageId << " " << newLeafPageId << std::endl;
      else
          *Trace << "ROOTSPLIT IN nodes " << currentLeafPageId << " " << newLeafPageId << std::endl;
      Trace->flush();
    }
#endif
    
    Datatype     tmpDatatype;
    RID          tmpRid;
    Keytype      tmpKey, oldKey;
    RID	         firstRid;
    int		 middle_split=0;	// to decide where to split the page
    int		 flag=1;

    oldKey.intkey = -1;		// should be some unused key value
    for (st = currentLeafPage->get_first(firstRid, (void*)&tmpKey, tmpRid);
	 st != NOMORERECS;
	 st = currentLeafPage->get_first(firstRid, (void*)&tmpKey, tmpRid))
    {
      RID insertRid;

      st = newLeafPage->insertRec(&tmpKey, headerPage->key_type,
				   tmpRid, insertRid);
      if (st != OK) // error will be handled in higher level
	return st;

      st = currentLeafPage->deleteRecord(firstRid);
      if (st != OK) // error will be handled in higher level
	return st;

      // just to make sure that records having the same key 
      // don't fall in different pages, middle_split will be set to
      // the appropriate point.
      if ((currentLeafPage->available_space() > newLeafPage->available_space())&&(flag==1)){
      	if ((keyCompare((void*)&oldKey, (void*)&tmpKey, headerPage->key_type)!=0))
     		flag = 0;
      	else
    		middle_split++;
      }
      if (headerPage->key_type == attrInteger)
      	oldKey.intkey = tmpKey.intkey;
    }

    assert(currentLeafPage->empty());
    assert(!newLeafPage->empty());

    // ASSERTIONS:
    // - currentLeafPage empty
    // - newLeafPage holds all former records from currentLeafPage


    int dummy_i = 0;
    for (st = newLeafPage->get_first(firstRid, (void*)&tmpKey, tmpRid);
	 (currentLeafPage->available_space() > newLeafPage->available_space())||(dummy_i < middle_split);
	 st = newLeafPage->get_first(firstRid, (void*)&tmpKey, tmpRid))
    {
		     // get the record from the current Leaf page
      CHECK_RAISE_ERROR(st, BTREE, CANT_SPLIT_LEAF_PAGE);

		     // now insert the <key,pageId> pair on the new Leaf page
      RID insertRid;
      st = currentLeafPage->insertRec((void*)&tmpKey, headerPage->key_type,
				      tmpRid, insertRid);

      st = newLeafPage->deleteRecord(firstRid);
      if (st != OK) // error will be handled in higher level
	return st;
      
      if (currentLeafPage->available_space() < newLeafPage->available_space())
	dummy_i++;
    }

     // check whether <key, rid>
     // will be inserted
     // on the newly allocated or on the old index page

    st = newLeafPage->get_first(firstRid, (void*)&tmpKey, tmpRid);
    CHECK_RAISE_ERROR(st, BTREE, CANT_SPLIT_LEAF_PAGE);

    if (keyCompare(key,(void*)&tmpKey, headerPage->key_type) >= 0)
    {		     // the new data entry belongs on the new Leaf page
      st = newLeafPage->insertRec(key, headerPage->key_type,
				  rid, tmpRid);
      CHECK_RAISE_ERROR(st, BTREE, CANT_SPLIT_LEAF_PAGE);
      
#ifdef BT_TRACE
  if ( Trace )
    {
      *Trace << "PUTIN node " << newLeafPageId << std::endl;
      Trace->flush();
    }
#endif
      
    }
    else {
      st = currentLeafPage->insertRec(key, headerPage->key_type,
				      rid, tmpRid);
      CHECK_RAISE_ERROR(st, BTREE, CANT_SPLIT_LEAF_PAGE);

#ifdef BT_TRACE
  if ( Trace )
    {
      *Trace << "PUTIN node " << currentLeafPageId << std::endl;
      Trace->flush();
    }
#endif
      
    }

    st = MINIBASE_BM->unpinPage(currentLeafPageId, TRUE /* dirty */);
    CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
        
    // fill *goingUp
    st = newLeafPage->get_first(firstRid, (void*)&tmpKey, tmpRid);
    CHECK_RAISE_ERROR(st, BTREE, CANT_SPLIT_LEAF_PAGE);


    tmpDatatype.pageNo = newLeafPageId;
    make_entry(*goingUp, headerPage->key_type, (void*)&tmpKey,
	       INDEX, tmpDatatype, goingUpSize);

    st = MINIBASE_BM->unpinPage(newLeafPageId, TRUE /* dirty */);
    CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);


    // ASSERTIONS:
    // - no pages pinned
    // - *goingUp holds the valid KeyDataEntry which is to be inserted 
    // on the index page one level up
    break;
  }
  default:	// in case memory is scribbled upon & type is hosed
    assert(0);
  }

  return OK;
}

/*
 *  Status BTreeFile::Delete (const void *key, const RID rid) 
 * 
 * Remove specified data entry (<key, rid>) from an index.
 * Based on the value of headerPage->delete_fashion to use either naive delete
 * algorithm or full delete algorithm (involving merge & redistribution)
 */
Status BTreeFile::Delete(const void *key, const RID rid)
{
    if (headerPage->delete_fashion == FULL_DELETE) 
        return FullDelete(key, rid); 
    else // headerPage->delete_fashion == NAIVE_DELETE
        return NaiveDelete(key, rid);
}

/*
 *  Status BTreeFile::NaiveDelete (const void *key, const RID rid) 
 * 
 * Remove specified data entry (<key, rid>) from an index.
 *
 * We don't do merging or redistribution, but do allow duplicates.
 *
 * Page containing first occurrence of key `key' is found for us
 * by findRunStart.  We then iterate for (just a few) pages, if necesary,
 * to find the one containing <key,rid>, which we then delete via
 * BTLeafPage::delUserRid.
 */

Status BTreeFile::NaiveDelete (const void *key, const RID rid)
{
  BTLeafPage *leafp;
  RID curRid;  // iterator
  Status st;
  Keytype curkey;
  RID dummyRid; 
  PageId nextpage;
  bool deleted;

#ifdef BT_TRACE
  if ( Trace )
    {
      *Trace << "DELETE " << rid.pageNo << " " << rid.slotNo << " " << (char*)key << std::endl;
      *Trace << "DO" << std::endl;
      *Trace << "SEARCH" << std::endl;
      Trace->flush();
    }
#endif

  st = findRunStart(key, &leafp, &curRid);  // find first page,rid of key
  CHECK_RAISE_ERROR(st, BTREE, DELETE_DATAENTRY_FAILED);


  leafp->get_current(curRid, &curkey, dummyRid);
  while (keyCompare(key, &curkey, headerPage->key_type) == 0) {


    deleted = leafp->delUserRid(key, headerPage->key_type, rid);
    if (deleted) {
      // successfully found <key, rid> on this page and deleted it.
      // unpin dirty page and return OK.
      
      st = MINIBASE_BM->unpinPage(leafp->page_no(), TRUE /* = DIRTY */);
      if (st != OK) {
	ADD_ERROR(BTREE, CANT_UNPIN_PAGE);
	RAISE_ERROR(BTREE, DELETE_DATAENTRY_FAILED);
      }
      
#ifdef BT_TRACE
  if ( Trace )
    {
      *Trace << "TAKEFROM node " << leafp->page_no() << std::endl;
      *Trace << "DONE\n";
      Trace->flush();
    }
#endif

      return OK;
    }

    nextpage = leafp->getNextPage();
    st = MINIBASE_BM->unpinPage(leafp->page_no());
    if (st != OK) {
      ADD_ERROR(BTREE, CANT_UNPIN_PAGE);
      RAISE_ERROR(BTREE, DELETE_DATAENTRY_FAILED);
    }

    st = MINIBASE_BM->pinPage(nextpage, (Page *&) leafp);
    if (st != OK) {
      ADD_ERROR(BTREE, CANT_PIN_PAGE);
      RAISE_ERROR(BTREE, DELETE_DATAENTRY_FAILED);
    }

    leafp->get_first(curRid, &curkey, dummyRid);
  }

  /*
   * We reached a page with first key > `key', so return an error.
   * We should have got true back from delUserRid above.  Apparently
   * the specified <key,rid> data entry does not exist.
   */
  
  st = MINIBASE_BM->unpinPage(leafp->page_no());
  CHECK_ADD_ERROR(st, BTREE, CANT_UNPIN_PAGE);
  RAISE_ERROR(BTREE, DELETE_DATAENTRY_FAILED);
}

/*
 * Status BTreeFile::FullDelete (const void *key, const RID rid) 
 * 
 * Remove specified data entry (<key, rid>) from an index.
 *
 * Most work done recursively by _Delete
 *
 * Special case: delete root if the tree is empty
 *
 * Page containing first occurrence of key `key' is found for us
 * After the page containing first occurence of key 'key' is found,
 * we iterate for (just a few) pages, if necesary,
 * to find the one containing <key,rid>, which we then delete via
 * BTLeafPage::delUserRid.
 */

Status BTreeFile::FullDelete (const void *key, const RID rid)
{
    Keytype oldChildKey;
    void *oldChildKeyPtr = &oldChildKey;

#ifdef BT_TRACE
  if ( Trace )
    {
    *Trace << "DELETE " << rid.pageNo << " " << rid.slotNo << " " << (char*)key << std::endl;
    *Trace << "DO" << std::endl;
    *Trace << "SEARCH" << std::endl;
    Trace->flush();
    }
#endif

    Status st = _Delete(key, rid, oldChildKeyPtr, headerPage->root, -1);

#ifdef BT_TRACE
  if ( Trace )
    {
    *Trace << "DONE\n";
    Trace->flush();
    }
#endif

    return st;
}

Status BTreeFile::_Delete (const void    *key,
                           const RID     rid,
                           void          *&oldChildEntry,
                           PageId        currentPageId,
                           PageId        parentPageId)
{
  Status st;
  SortedPage*  rpPtr;

  assert(currentPageId != INVALID_PAGE);
  
  st = MINIBASE_BM->pinPage(currentPageId,(Page *&) rpPtr);
  CHECK_RAISE_ERROR(st, BTREE, CANT_PIN_PAGE);


  nodetype pageType = (nodetype)rpPtr->get_type();

#ifdef BT_TRACE
  if ( Trace )
    {
      *Trace << "VISIT node " << currentPageId << std::endl;
      Trace->flush();
    }
#endif

  switch(pageType) {
      case LEAF: 
          RID curRid;  // iterator
          Keytype curkey;
          RID dummyRid; 
          PageId nextpage;
          bool deleted;

          // find the first record with key equal to 'key'
          st = ((BTLeafPage*)rpPtr)->get_first(curRid, (void*)&curkey, dummyRid);
          if (st == NOMORERECS) {
              st = MINIBASE_BM->unpinPage(rpPtr->page_no());
              CHECK_ADD_ERROR(st, BTREE, CANT_UNPIN_PAGE);
              RAISE_ERROR(BTREE, DELETE_DATAENTRY_FAILED);
          }
          while (keyCompare(key, &curkey, headerPage->key_type) > 0) {
              st = ((BTLeafPage*)rpPtr)->get_next(curRid, (void*)&curkey, dummyRid);
              if (st == NOMORERECS) {
                  st = MINIBASE_BM->unpinPage(rpPtr->page_no());
                  CHECK_ADD_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                  RAISE_ERROR(BTREE, DELETE_DATAENTRY_FAILED);
              }
          }
          if (keyCompare(key, &curkey, headerPage->key_type) != 0) {
              // record to be deleted not found 
              st = MINIBASE_BM->unpinPage(rpPtr->page_no());
              CHECK_ADD_ERROR(st, BTREE, CANT_UNPIN_PAGE);
              RAISE_ERROR(BTREE, DELETE_DATAENTRY_FAILED);
          }

          // save possible old child entry before deletion
          Keytype oldEntry;
          st = ((BTLeafPage*)rpPtr)->get_first(curRid, (void*)&oldEntry, dummyRid);
          assert(st == OK);

          const void *deletedKey;
          deletedKey = NULL;
          if (keyCompare(key, &curkey, headerPage->key_type) == 0)
              // the first entry on the leaf page is deleted
              // it should be stored in case its entry in the parent needs adjusted
              // after redistribution performed later
              deletedKey = key;
          
          // for all records with key equal to 'key', delete it if its rid = 'rid'
          while (keyCompare(key, &curkey, headerPage->key_type) == 0) {
	      // WriteUpdateLog is done in the btleafpage level - to log the
	      // deletion of the rid.
              deleted = ((BTLeafPage*)rpPtr)->delUserRid(key, headerPage->key_type, rid);
              if (deleted) {
                  // successfully found <key, rid> on this page and deleted it.

#ifdef BT_TRACE
                  if ( Trace )
                    {
                      *Trace << "TAKEFROM node " << rpPtr->page_no() << std::endl;
                      Trace->flush();
                    }
#endif
                  
                  if (rpPtr->free_space() <= (MAX_SPACE-DPFIXED)/2) { 
                      // the leaf page is at least half full after the deletion
                      oldChildEntry = NULL;
                      st = MINIBASE_BM->unpinPage(rpPtr->page_no(), TRUE /* = DIRTY */);
                      CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                      return OK;
                  }
                  else if (rpPtr->page_no() == headerPage->root) {
                      // the tree has only one node - the root
                      if (rpPtr->numberOfRecords() != 0) {
                          oldChildEntry = NULL;
                          st = MINIBASE_BM->unpinPage(rpPtr->page_no(), TRUE /* = DIRTY */);
                          CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                          return OK;
                      }
                      else {
                          // the whole tree is empty
#ifdef BT_TRACE
                          if ( Trace )
                            {
                              *Trace << "DEALLOCATEROOT " << rpPtr->page_no() << std::endl;
                              Trace->flush();
                            }
#endif
                          oldChildEntry = NULL;
                          st = MINIBASE_BM->freePage(rpPtr->page_no());
                          CHECK_RAISE_ERROR(st, BTREE, CANT_FREE_PAGE);
                          updateHeader(INVALID_PAGE);
                          return OK;
                      }
                  }
                  else {
                      // get a sibling
                      BTIndexPage * parentPtr;
                      st = MINIBASE_BM->pinPage(parentPageId,(Page *&) parentPtr);
                      CHECK_RAISE_ERROR(st, BTREE, CANT_PIN_PAGE);
                      PageId siblingPageId;
                      BTLeafPage *siblingPtr;
                      int left;

                      if (!parentPtr->get_sibling(key, headerPage->key_type, siblingPageId, left)) {
                          // there is no sibling. nothing can be done.
                          oldChildEntry = NULL;
                          st = MINIBASE_BM->unpinPage(rpPtr->page_no(), TRUE /* = DIRTY */);
                          CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                          st = MINIBASE_BM->unpinPage(parentPageId);
                          CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                          return OK;
                      }
                      
                      st = MINIBASE_BM->pinPage(siblingPageId, (Page* &) siblingPtr);
                      CHECK_RAISE_ERROR(st, BTREE, CANT_PIN_PAGE);
                      if (siblingPtr->redistribute((BTLeafPage*)rpPtr, parentPtr, headerPage->key_type, left, deletedKey)) {
                          // the redistribution has been done successfully

                          oldChildEntry = NULL;
                          
                          st = MINIBASE_BM->unpinPage(rpPtr->page_no(), TRUE);
                          CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                          st = MINIBASE_BM->unpinPage(siblingPageId, TRUE);
                          CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                          st = MINIBASE_BM->unpinPage(parentPageId, TRUE);
                          CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);

                          return OK;
                      }
                      else if (siblingPtr->free_space() >=
                          (MAX_SPACE-DPFIXED) - rpPtr->free_space()) {
                          // we can merge these two children

                          // get old child entry in the parent first
                          if (left)
                              memcpy(oldChildEntry, &oldEntry, headerPage->keysize);
                          else {
                              st = siblingPtr->get_first(curRid, oldChildEntry, dummyRid);
                              assert(st == OK);
                          }

                          // merge the two children
                          BTLeafPage *leftChild, *rightChild;
                          if (left) {
                              leftChild = siblingPtr;
                              rightChild = (BTLeafPage*)rpPtr;
                          }
                          else {
                              leftChild = (BTLeafPage*)rpPtr;
                              rightChild = siblingPtr;
                          }

                          // move all entries from rightChild to leftChild
                          RID firstRid, insertRid;
                          for (st = rightChild->get_first(firstRid, &curkey, curRid);
                               st != NOMORERECS;
                               st = rightChild->get_first(firstRid, &curkey, curRid)) {
                              st = leftChild->insertRec(&curkey, headerPage->key_type, curRid, insertRid);
                              assert(st == OK);

                              st = rightChild->deleteRecord(firstRid);
                              assert(st == OK);
                          }
                          assert(rightChild->empty());

                          // adjust the leaf chain
                          leftChild->setNextPage(rightChild->getNextPage());

#ifdef BT_TRACE
                          if ( Trace )
                            {
                              *Trace << "MERGE nodes " << leftChild->page_no() << " "
                                     << rightChild->page_no() << std::endl;
                              Trace->flush();
                            }
#endif
                                                    
                          st = MINIBASE_BM->unpinPage(leftChild->page_no(), TRUE);
                          CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                          st = MINIBASE_BM->unpinPage(parentPageId, TRUE);
                          CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                          st = MINIBASE_BM->freePage(rightChild->page_no());
                          CHECK_RAISE_ERROR(st, BTREE, CANT_FREE_PAGE);

                          return OK;
                      }
                      else {
                          // It's a very rare case when we can do neither
                          // redistribution nor merge. 
                          oldChildEntry = NULL;

                          st = MINIBASE_BM->unpinPage(rpPtr->page_no(), TRUE);
                          CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                          st = MINIBASE_BM->unpinPage(siblingPageId, TRUE);
                          CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                          st = MINIBASE_BM->unpinPage(parentPageId, TRUE);
                          CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);

                          return OK;
                      }
                  }
              }

              nextpage = rpPtr->getNextPage();
              st = MINIBASE_BM->unpinPage(rpPtr->page_no());
              if (st != OK) {
                  ADD_ERROR(BTREE, CANT_UNPIN_PAGE);
                  RAISE_ERROR(BTREE, DELETE_DATAENTRY_FAILED);
              }

              st = MINIBASE_BM->pinPage(nextpage, (Page *&) rpPtr);
              if (st != OK) {
                  ADD_ERROR(BTREE, CANT_PIN_PAGE);
                  RAISE_ERROR(BTREE, DELETE_DATAENTRY_FAILED);
              }

              ((BTLeafPage*)rpPtr)->get_first(curRid, &curkey, dummyRid);
          }

          /*
           * We reached a page with first key > `key', so return an error.
           * We should have got true back from delUserRid above.  Apparently
           * the specified <key,rid> data entry does not exist.
           */
  
          st = MINIBASE_BM->unpinPage(rpPtr->page_no());
          CHECK_ADD_ERROR(st, BTREE, CANT_UNPIN_PAGE);
          RAISE_ERROR(BTREE, DELETE_DATAENTRY_FAILED);

          break;

      case INDEX:
          PageId childPageId;
          st = ((BTIndexPage*)rpPtr)->get_page_no(key, headerPage->key_type, childPageId);
          assert(childPageId != INVALID_PAGE);
          CHECK_RAISE_ERROR(st, BTREE, CANT_GET_PAGE_NO);

          // now unpin the page, recurse and then pin it again
          st = MINIBASE_BM->unpinPage(currentPageId);
          CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);

          st = _Delete(key, rid, oldChildEntry, childPageId, currentPageId);
          if (st != OK) // error will be handled on higher level
              return st;

          // two cases:
          // - oldChildEntry == NULL: one level lower no merge has occurred:
          // - oldChildEntry != NULL: one of the children has been deleted and
          //                     oldChildEntry is the entry to be deleted.

          st = MINIBASE_BM->pinPage(currentPageId,(Page *&) rpPtr);
          CHECK_RAISE_ERROR(st, BTREE, CANT_PIN_PAGE);

          if (!oldChildEntry) {
              st = MINIBASE_BM->unpinPage(rpPtr->page_no(),TRUE);
              CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
              return OK;
          }

          // delete the oldChildEntry

          // save possible old child entry before deletion
          PageId dummyPageId;
          st = ((BTIndexPage*)rpPtr)->get_first(curRid, (void*)&oldEntry, dummyPageId);
          assert(st == OK);

          deletedKey = NULL;
          st = ((BTIndexPage*)rpPtr)->deleteKey(oldChildEntry,headerPage->key_type,curRid);
          assert(st == OK);

          if (curRid.slotNo == 0) 
              // the first entry on the index page is deleted
              // it should be stored in case its entry in the parent needs adjusted
              // after redistribution performed later
              deletedKey = key;
          
          if (rpPtr->page_no() == headerPage->root) {
              // the index page is the root
              if (rpPtr->numberOfRecords() == 0) {
                  SortedPage *childPtr;
                  st = MINIBASE_BM->pinPage(rpPtr->getPrevPage(),(Page*&)childPtr);
                  CHECK_RAISE_ERROR(st, BTREE, CANT_PIN_PAGE);

#ifdef BT_TRACE
                  if ( Trace )
                    {
                      *Trace << "CHANGEROOT from node " << rpPtr->page_no()
                             << " to node " << rpPtr->getPrevPage() << std::endl;
                      Trace->flush();
                    }
#endif
                          
                  st = updateHeader(rpPtr->getPrevPage());
                  if (st != OK)
                      return st;
                  st = MINIBASE_BM->unpinPage(childPtr->page_no());
                  CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                  st = MINIBASE_BM->freePage(rpPtr->page_no());
                  CHECK_RAISE_ERROR(st, BTREE, CANT_FREE_PAGE);
                  oldChildEntry = NULL;
                  return OK;

              }
              st = MINIBASE_BM->unpinPage(rpPtr->page_no(),TRUE);
              CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
              oldChildEntry = NULL;
              return OK;
          }

          // now we know the current index page is not a root
          if (rpPtr->free_space() <= (MAX_SPACE-DPFIXED)/2) {
              // the index page is at least half full after the deletion
              st = MINIBASE_BM->unpinPage(currentPageId,TRUE);
              CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
              oldChildEntry = NULL;
              return OK;
          }
          else {
              // get a sibling
              BTIndexPage * parentPtr;
              st = MINIBASE_BM->pinPage(parentPageId,(Page *&)parentPtr);
              CHECK_RAISE_ERROR(st, BTREE, CANT_PIN_PAGE);
              PageId siblingPageId;
              BTIndexPage *siblingPtr;
              int left;
              if (!parentPtr->get_sibling(key, headerPage->key_type, siblingPageId, left)) {
                  // there is no sibling. nothing can be done.
                  oldChildEntry = NULL;
                  st = MINIBASE_BM->unpinPage(rpPtr->page_no(), TRUE);
                  CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                  st = MINIBASE_BM->unpinPage(parentPageId);
                  CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                  return OK;
              }
              
              st = MINIBASE_BM->pinPage(siblingPageId, (Page* &) siblingPtr);
              CHECK_RAISE_ERROR(st, BTREE, CANT_PIN_PAGE);


              if (siblingPtr->redistribute((BTIndexPage*)rpPtr,parentPtr, headerPage->key_type, left, deletedKey)) {
                  // the redistribution has been done successfully

#ifdef BT_TRACE
                  trace_children(rpPtr->page_no());
                  trace_children(siblingPtr->page_no());
#endif
    
                  oldChildEntry = NULL;
                  
                  st = MINIBASE_BM->unpinPage(rpPtr->page_no(), TRUE);
                  CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                  st = MINIBASE_BM->unpinPage(siblingPageId, TRUE);
                  CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                  st = MINIBASE_BM->unpinPage(parentPageId, TRUE);
                  CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);

                  return OK;
              }
              else if ((unsigned int) siblingPtr->free_space() >=
                       (MAX_SPACE-DPFIXED) - rpPtr->free_space()
                    + sizeof(slot_t) + headerPage->keysize + sizeof(PageId)) {
                  // we can merge these two children 

                  // get old child entry in the parent first
                  if (left)
                      memcpy(oldChildEntry, &oldEntry, headerPage->keysize);
                  else {
                      st = siblingPtr->get_first(curRid, oldChildEntry, dummyPageId);
                      assert(st == OK);
                  }

                  // merge the two children
                  BTIndexPage *leftChild, *rightChild;
                  if (left) {
                      leftChild = siblingPtr;
                      rightChild = (BTIndexPage*)rpPtr;
                  }
                  else {
                      leftChild = (BTIndexPage*)rpPtr;
                      rightChild = siblingPtr;
                  }

#ifdef BT_TRACE
                  if ( Trace )
                    {
                      *Trace << "MERGE nodes " << leftChild->page_no() << " "
                             << rightChild->page_no() << std::endl;
                      Trace->flush();
                    }
#endif

                  // pull down the entry in its parent node
                  // and put it at the end of the left child
                  RID firstRid, insertRid;
                  PageId curPageId;

                  st = parentPtr->findKey(oldChildEntry, &curkey, headerPage->key_type);
                  assert(st == OK);
                  curPageId = rightChild->getLeftLink();
                  st = leftChild->insertKey(&curkey, headerPage->key_type, curPageId, insertRid);
                  assert(st == OK);
                  
                  // move all entries from rightChild to leftChild
                  for (st = rightChild->get_first(firstRid, &curkey, curPageId);
                       st != NOMORERECS;
                       st = rightChild->get_first(firstRid, &curkey, curPageId)) {
                      st = leftChild->insertKey(&curkey, headerPage->key_type, curPageId, insertRid);
                      assert(st == OK);
                      
                      st = rightChild->deleteRecord(firstRid); 
                      assert(st == OK);
                  }
                  assert(rightChild->empty());

                  st = MINIBASE_BM->unpinPage(leftChild->page_no(), TRUE);
                  CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                  st = MINIBASE_BM->unpinPage(parentPageId, TRUE);
                  CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                  st = MINIBASE_BM->freePage(rightChild->page_no());
                  CHECK_RAISE_ERROR(st, BTREE, CANT_FREE_PAGE);

                  memcpy(oldChildEntry, &curkey, headerPage->keysize);
                  return OK; 

              }
              else {
                  // It's a very rare case when we can do neither
                  // redistribution nor merge. 
                  oldChildEntry = NULL;

                  st = MINIBASE_BM->unpinPage(rpPtr->page_no(), TRUE);
                  CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                  st = MINIBASE_BM->unpinPage(siblingPageId, TRUE);
                  CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
                  st = MINIBASE_BM->unpinPage(parentPageId);
                  CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);

                  return OK;
              }
          }
  }
  return OK;
}

/*
 * IndexFileScan* BTreeFile::new_scan (const void *lo_key, const void *hi_key)
 *
 * create a scan with given keys
 * Cases:
 *      (1) lo_key = NULL, hi_key = NULL
 *              scan the whole index
 *      (2) lo_key = NULL, hi_key!= NULL
 *              range scan from min to the hi_key
 *      (3) lo_key!= NULL, hi_key = NULL
 *              range scan from the lo_key to max
 *      (4) lo_key!= NULL, hi_key!= NULL, lo_key = hi_key
 *              exact match ( might not unique)
 *      (5) lo_key!= NULL, hi_key!= NULL, lo_key < hi_key
 *              range scan from lo_key to hi_key   
 *
 * The work of finding the first page to scan is done by findRunStart (below).
 */

IndexFileScan* BTreeFile::new_scan (const void *lo_key, const void *hi_key)
{
  Status st;

  BTreeFileScan *scanp = new BTreeFileScan;
  if (headerPage->root == INVALID_PAGE) {
    // tree is empty, so return a scan object that will iterate zero times.
    scanp->leafp = NULL;
    return scanp;
  }

  scanp->treep = this;
  scanp->endkey = hi_key;  // XXX may need to copy data over

  scanp->didfirst = false;
  scanp->deletedcurrent = false;

  // this sets up scanp at starting position, ready for iteration:
  st = findRunStart(lo_key, &scanp->leafp, &scanp->curRid);
  if (st != OK) { 
    // error (if any) has already been registered by findScanStart
    scanp->leafp = NULL; // for ~BTreeFileScan
    delete scanp;
    return NULL;
  }
  
  return scanp;
}


/* 
 * Status BTreeFile::findRunStart (const void   *lo_key,
 *				BTLeafPage  **pppage,
 *				RID          *pstartrid)
 *
 * find left-most occurrence of `lo_key', going all the way left if
 * lo_key is NULL.
 * 
 * Starting record returned in *pstartrid, on page *pppage, which is pinned.
 *
 * Since we allow duplicates, this must "go left" as described in the text
 * (for the search algorithm). 
 */

Status BTreeFile::findRunStart (const void   *lo_key,
				BTLeafPage  **pppage,
				RID          *pstartrid)
{
  BTLeafPage *ppage;
  BTIndexPage *ppagei;
  PageId pageno;
  PageId curpage;		// iterator
  PageId prevpage;
  PageId nextpage;
  RID metaRid, curRid;
  Keytype curkey;
  Status st;
  AttrType key_type = headerPage->key_type;

  pageno = headerPage->root;
  if (pageno == INVALID_PAGE){	// no pages in the BTREE
	*pppage = NULL;				// should be handled by 
	pstartrid = NULL;			// the caller
	return OK;
  }
  st = MINIBASE_BM->pinPage(pageno, (Page *&) ppagei);
  CHECK_RAISE_ERROR(st, BTREE, CANT_PIN_PAGE);


#ifdef BT_TRACE
  if ( Trace )
    {
      *Trace << "VISIT node " << pageno << std::endl;
      Trace->flush();
    }
#endif
  
  // ASSERTION
  // - pageno and ppagei is the root of the btree
  // - pageno and ppagei valid and pinned

  while (ppagei->get_type() == INDEX) {

    prevpage = ppagei->getPrevPage();
    st = ppagei->get_first(metaRid, &curkey, curpage);
    while (st == OK && lo_key && keyCompare(&curkey, lo_key, key_type) < 0) {


      prevpage = curpage;
      st = ppagei->get_next(metaRid, &curkey, curpage);
    }

    st = MINIBASE_BM->unpinPage(pageno);
    CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);

    pageno = prevpage;
    st = MINIBASE_BM->pinPage(pageno, (Page *&) ppagei);
    CHECK_RAISE_ERROR(st, BTREE, CANT_PIN_PAGE);

#ifdef BT_TRACE
  if ( Trace )
    {
      *Trace << "VISIT node " << pageno << std::endl;
      Trace->flush();
    }
#endif
    
  }

  assert(ppagei && ppagei->get_type() == LEAF);
  ppage = (BTLeafPage *) ppagei;  

  st = ppage->get_first(metaRid, &curkey, curRid);
  while (st == NOMORERECS) {
    // skip empty leaf pages off to left
    nextpage = ppage->getNextPage();
    st = MINIBASE_BM->unpinPage(pageno);
    CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);
    if (nextpage == INVALID_PAGE) {
      // oops, no more records, so set this scan to indicate this.
      *pppage = NULL;
      return OK;
    }

    pageno = nextpage;
    st = MINIBASE_BM->pinPage(pageno, (Page *&) ppage);

    CHECK_RAISE_ERROR(st, BTREE, CANT_PIN_PAGE);

    
    st = ppage->get_first(metaRid, &curkey, curRid);
  }

  // ASSERTIONS:
  // - curkey, curRid: contain the first record on the
  //     current leaf page (curkey its key, cur
  // - ppage, pageno valid and pinned
 

  if (lo_key == NULL) {
    *pppage = ppage;
    *pstartrid = metaRid;
    return OK;
    // note that pageno/ppage is still pinned; scan will unpin it when done
  }
  
  while (keyCompare(&curkey, lo_key, key_type) < 0) {
    st = ppage->get_next(metaRid, &curkey, curRid);
    while (st == NOMORERECS) { // have to go right
      nextpage = ppage->getNextPage();
      st = MINIBASE_BM->unpinPage(pageno);
      CHECK_RAISE_ERROR(st, BTREE, CANT_UNPIN_PAGE);

      if (nextpage == INVALID_PAGE) {
	*pppage = NULL;
	return OK;
      }

      pageno = nextpage;
      st = MINIBASE_BM->pinPage(pageno, (Page *&) ppage);
      CHECK_RAISE_ERROR(st, BTREE, CANT_PIN_PAGE);

      st = ppage->get_first(metaRid, &curkey, curRid);
    }
  }


  *pppage = ppage;
  *pstartrid = metaRid;

  return OK;
}

int BTreeFile::keysize()
{
  return headerPage->keysize;
}




void BTreeFile::PrintHeader()
{
  std::cout << "\nPRINTING B-TREE HEADER PAGE-------------------------------\n";
  std::cout << "Header page info of BTree index " << dbname << " :" << std::endl;
  std::cout << "  Root page number : " << headerPage->root << std::endl;
  std::cout << "  Key type : ";
  switch (headerPage->key_type)
    {
    case attrInteger: std::cout << "Integer" << std::endl; break;
    case attrReal:    std::cout << "Real"    << std::endl; break;
    case attrString:  std::cout << "String"  << std::endl; break;
    default: break;
    }
  std::cout << "  Key size : " << headerPage->keysize << std::endl << std::endl;
}

void BTreeFile::PrintRoot()
{
  std::cout << "\nPRINTING B-TREE ROOT PAGE-------------------\n";
  if (headerPage->root != INVALID_PAGE)
  	PrintPage( headerPage->root );
}

void BTreeFile::PrintLeafPages()
{
  Status st;
  BTLeafPage *leafp;
  RID dummy;


  // Find first leaf node.
  st = findRunStart( NULL, &leafp, &dummy );
  if ( st != OK )
    {  
      std::cerr << "Error finding start of b-tree" << std::endl;
      return;
    }	
  
  while ( leafp )
    {
      PrintPage( leafp->page_no() );

      PageId next = leafp->getNextPage();
      st = MINIBASE_BM->unpinPage( leafp->page_no() );
      if (st != OK)
	{
	  ADD_ERROR(BTREE, BTreeFile::CANT_UNPIN_PAGE);
	  std::cerr << "Can't unpin a b-tree page" << std::endl;
	  return;
	}
      
      if ( next == INVALID_PAGE )
	leafp = NULL;
      else
	{
	  st = MINIBASE_BM->pinPage( next, (Page *&) leafp );
	  if (st != OK)
	    {
	      ADD_ERROR(BTREE, BTreeFile::CANT_PIN_PAGE);
	      std::cerr << "Can't pin a b-tree page" << std::endl;
	      return;
	    }
	}
    }
}

void BTreeFile::PrintPage(PageId id)
{
  Status st;
  SortedPage* page;

  st = MINIBASE_BM->pinPage(id, (Page*&)page );
  if ( st != OK )
    {
      std::cerr << "Error reading b-tree page #" << id << std::endl;
      return;
    }
  
  std::cout << "\n------------------------------------------------------\n";
  std::cout << "B-Tree page #" << id << std::endl;
  std::cout << "Number of records : " << page->numberOfRecords() << std::endl;
  
  if ( page->get_type() == LEAF )
    {
      std::cout << "Node type : Leaf" << std::endl;
      std::cout << "Right sibling : " << page->getNextPage() << std::endl;
      std::cout << "--------------records in the page------------------" << std::endl;

      BTLeafPage* leafp = (BTLeafPage*) page;
      RID metaRid, dataRid;
      Keytype key;
      for ( st = leafp->get_first( metaRid, &key, dataRid );
	    st == OK;
	    st = leafp->get_next( metaRid, &key, dataRid ) )
	{
        std::cout << "Page/slot: " << dataRid.pageNo << '/'
             << dataRid.slotNo << " Key: ";
        switch ( headerPage->key_type )
            {
                case attrString:    std::cout << key.charkey; break;
                case attrInteger:   std::cout << key.intkey;  break;
                default: break;
            }
        std::cout << std::endl;
    }
    }
  else
    {
      std::cout << "Node type : Internal" << std::endl;
      std::cout << "Left-most child : " << page->getPrevPage() << std::endl;
      std::cout << "--------------records in the page------------------" << std::endl;

      BTIndexPage* indexp = (BTIndexPage*) page;
      RID metaRid;
      PageId pg;
      Keytype key;
      for ( st = indexp->get_first( metaRid, &key, pg );
	    st == OK;
	    st = indexp->get_next( metaRid, &key, pg ) )
	{
        std::cout << "Page: " << pg << " Key: ";
	  switch ( headerPage->key_type )
	    {
	    case attrString:    std::cout << key.charkey; break;
	    case attrInteger:   std::cout << key.intkey;  break;
	    default: break;
	    }
        std::cout << std::endl;
	}
    }
  
  std::cout << "------------------end of page--------------------------\n\n";
  MINIBASE_BM->unpinPage(id);
}



#if defined(BT_TRACE)

#include <stream.h>
std::ostream* BTreeFile::Trace = 0;

void BTreeFile::trace_children(PageId id)
{
  *Trace << "CHILDREN " << id << " nodes";
  
  BTIndexPage* pg;
  RID metaRid;
  PageId childPageId;
  Keytype key;
  Status st;

  MINIBASE_BM->pinPage( id, (Page*&)pg );
  
  // Now print all the child nodes of the page.  (This is only called on
  // internal nodes.)
  *Trace << ' ' << pg->getPrevPage();
  for ( st = pg->get_first( metaRid, &key, childPageId );
        st == OK;
        st = pg->get_next( metaRid, &key, childPageId ) )
    {
      *Trace << ' ' << childPageId;
    }
  
  MINIBASE_BM->unpinPage( id );
  *Trace << std::endl;
  Trace->flush();
}

#endif
