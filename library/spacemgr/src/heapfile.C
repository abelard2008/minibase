/* 
 * heapfile.cc - implementation of class HeapFile
 *
 * Johannes Gehrke & Gideon Glass  950920  CS564  UW-Madison
 *
 * Modified by Michael Lee, 1995
 *
 * A heapfile is an unordered set of tuples stored on pages.
 * Free space on pages is managed via a directory structure.
 */

#include <stdio.h>
#include <assert.h>
#include "heapfile.h"
#include "hfpage.h"
#include "scan.h"
#include "buf.h"
#include "db.h"


#ifndef offsetof
#define offsetof(ty,mem) ((size_t)&(((ty*)0)->mem))
#endif


//******************************************************
// Error messages for the heapfile layer

static const char* hfErrMsgs[] = {
    "bad record id",
    "bad record pointer", 
    "end of file encountered",
    "invalid update operation",
    "no space on page for record", 
    "page is empty - no records",
    "last record on page",
    "invalid slot number",
    "file has already been deleted",
};

static error_string_table hfTable( HEAPFILE, hfErrMsgs );
extern "C" int getpid();




/*
 *  HeapFile::HeapFile (char *name, Status& returnStatus)
 *
 * if the heapfile does already exist in the database, get the
 *   first directory page and pin it.
 * if the heapfile does not yet exist, create it, get the first
 *   directory page and pin it.
 */
HeapFile::HeapFile( const std::string name, Status& returnStatus )
{
     // Give us a prayer of destructing cleanly if construction fails.
    _file_deleted = true;
    _fileName = 0;

      // If the name is NULL, allocate a temporary name and no logging is
      // required
    if ( name.empty())
      {
        char tempname[MAX_NAME];
        static unsigned numtemps;
        sprintf( tempname, "*** temp HeapFile #%d:%u ***",
                 getpid(), numtemps++ );
        _fileName = strdup( tempname );
        _ftype = TEMP;
      }
    else
      {
        _fileName = strdup( name.c_str() );
        _ftype = ORDINARY;
      }

    Status status;
    Page*  pagePtr;

      // The constructor gets run in two different cases.
      // In the first case, the file is new and the header page
      // must be initialized.  This case is detected via a failure
      // in the db->get_file_entry() call.  In the second case, the
      // file already exists and all that must be done is to fetch
      // the header page into the buffer pool

      // try to open the file

    if (_ftype == ORDINARY)
        status = MINIBASE_DB->get_file_entry(_fileName, _firstDirPageId);
    else
        status = DBMGR;


    if (status != OK)
      {
          // file doesn't exist. First create it.
        status = MINIBASE_BM->newPage(_firstDirPageId, pagePtr);
        if (status != OK) 
          {
#ifdef DEBUG
            std::cerr << "allocation of header page failed\n";
#endif
            returnStatus = MINIBASE_CHAIN_ERROR( HEAPFILE, status );
            return;
          }

        status = MINIBASE_DB->add_file_entry(_fileName, _firstDirPageId);
        if (status != OK) 
          {
#ifdef DEBUG
            std::cerr << "Could not add file entry \n";
#endif
            returnStatus = status;
            return;
          }

	HFPage* firstDirPage = (HFPage*) pagePtr;
        firstDirPage->init(_firstDirPageId);
        firstDirPage->setNextPage(INVALID_PAGE);
        firstDirPage->setPrevPage(INVALID_PAGE);
        status = MINIBASE_BM->unpinPage(_firstDirPageId, true /*dirty*/ );
        if (status != OK) 
          {
#ifdef DEBUG
            std::cerr << "unpinning of new header page failed\n";
#endif
            returnStatus = MINIBASE_CHAIN_ERROR( HEAPFILE, status );
            return;
          }
    } 

    _file_deleted = false;
    returnStatus = OK;
   
    // ASSERTIONS:
    // - ALL private data members of class Heapfile are valid:
    //
    // 	- _firstDirPageId valid
    //	- _fileName valid
    // - no datapage pinned yet
 
}

//******************************************************************
// Destructor

HeapFile::~HeapFile()
{


    // ASSERTIONS:
    // - no pages are pinned
    // - private members of class Heapfile are valid


    delete _fileName;

    if ( _ftype == TEMP && !_file_deleted )
      {
#ifdef DEBUG
        Status status =
#endif
            deleteFile();
#ifdef DEBUG
        if ( status != OK )
            std::cerr << "Error in deleting temporary file" << endl;
#endif
        return;
      }

    if (!_file_deleted) {

#ifdef MULTI_USER
	_hf_pgid = _firstDirPageId;
        status = MINIBASE_LOCKMGR->lock_page(_hf_pgid, Exclusive);
	if (status != OK) {
            return MINIBASE_CHAIN_ERROR( HEAPFILE, status );
	}
        status = MINIBASE_BM->unpinPage(headerPageNo, TRUE);

	if (status != OK) {
            return MINIBASE_CHAIN_ERROR( HEAPFILE, status );
	}

#endif

    }
}


//********************************************************************

// Wipes out the heapfile from the database permanently.  This function
// is also used by the destructor for temporary files.

Status HeapFile::deleteFile()
{
      // If file has already been deleted, return an error status
    Status status;

    if ( _file_deleted )
        return MINIBASE_FIRST_ERROR( HEAPFILE, ALREADY_DELETED );


      // Mark the deleted flag (even if it doesn't get all the way done).
    _file_deleted = true;


      // Deallocate all data pages
    PageId currentDirPageId = _firstDirPageId, nextDirPageId = 0;
    HFPage* currentDirPage;
    status = MINIBASE_BM->pinPage(currentDirPageId, (Page*&)currentDirPage);
    if ( status != OK )
        return MINIBASE_CHAIN_ERROR( HEAPFILE, status );

    while (currentDirPageId != INVALID_PAGE) {
        RID rid;
	for (status = currentDirPage->firstRecord(rid);
	     status == OK;
	     status = currentDirPage->nextRecord (rid, rid)) {
	    
            struct DataPageInfo dpinfo;
            int dpinfoLen;

	    Status st = currentDirPage->getRecord(rid, (char *) &dpinfo, dpinfoLen);
	    if (st != OK)
		return MINIBASE_CHAIN_ERROR( HEAPFILE, st );

	    st = MINIBASE_BM->freePage(dpinfo.pageId);
	    if (st != OK)
		return MINIBASE_CHAIN_ERROR( HEAPFILE, st );
	}
	
	// ASSERTIONS:
	// - we have freePage()'d all data pages referenced by the current
	// directory page.
	
	nextDirPageId = currentDirPage->getNextPage();
	status = MINIBASE_BM->freePage(currentDirPageId);
	if (status != OK)
	    return MINIBASE_CHAIN_ERROR( HEAPFILE, status );

	currentDirPageId = nextDirPageId;
	if (nextDirPageId != INVALID_PAGE) {
	    status = MINIBASE_BM->pinPage(currentDirPageId, (Page *&) currentDirPage);
	    if (status != OK)
		return MINIBASE_CHAIN_ERROR( HEAPFILE, status );
	}
    }


      // Deallocate the file entry and header page.
    status = MINIBASE_DB->delete_file_entry( _fileName );
    if ( status != OK )
        return MINIBASE_CHAIN_ERROR( HEAPFILE, status );

    return OK;
}

//********************************************************************
    
// Return number of records in heap file

int HeapFile::getRecCnt()
{
    Status status = OK;
    int answer = 0;
    PageId currentDirPageId = _firstDirPageId, nextDirPageId = 0;
    HFPage* currentDirPage;

    while ( status == OK && currentDirPageId != INVALID_PAGE )
      {

        status = MINIBASE_BM->pinPage(currentDirPageId,(Page*&)currentDirPage);
        if ( status != OK )
            break;


        RID rid;
        for ( status = currentDirPage->firstRecord(rid);
              status == OK;
              status = currentDirPage->nextRecord(rid, rid))
          {
            DataPageInfo dpinfo;
            int dpinfoLen;
            Status st = currentDirPage->getRecord(rid, (char *) &dpinfo, dpinfoLen);
            if (st != OK )
              {
                status = st;
                break;
              }

            answer += dpinfo.recct;
          }

        if ( status == DONE )
          {

              // ASSERTIONS:
              // - we have read all datapage records on the current directory page

            nextDirPageId = currentDirPage->getNextPage();
            status = OK;
          }

          // the current directory page is not used any more; it was not modified
        Status st = MINIBASE_BM->unpinPage( currentDirPageId );

        if ( status == OK )
          {
            currentDirPageId = nextDirPageId;
            status = st;
          }

          // ASSERTIONS:
          // - if error, status != OK; else:
          // - if end of heapfile reached: currentDirPageId == INVALID_PAGE
          // - if not yet end of heapfile: currentDirPageId valid
      }

    if ( status != OK )
      {
        MINIBASE_CHAIN_ERROR( HEAPFILE, status );
        answer = -1;
      }

    return answer;
}



// -----------------------------------------------------------------------
// get a new datapage from the buffer manager and initialize dpinfo
// returns a pointer to the newly allocated page
// remark:
//   the insertion of this datapage and its datapageinfo into the
//   directory structure of the heapfile is not done by this function
HFPage *HeapFile::_newDatapage(DataPageInfo *dpinfop)
{
  HFPage *pagep;

  // ASSERTIONS: none

  Status st = MINIBASE_BM ->newPage(dpinfop->pageId, (Page *&) pagep);
  
  if (st != OK) {
    return NULL;
  }

  // initialize internal values of the new page:


  pagep->init(dpinfop->pageId);  
  pagep->setNextPage(INVALID_PAGE);
  pagep->setPrevPage(INVALID_PAGE);
  
  dpinfop->recct = 0;
  dpinfop->availspace = pagep->available_space();

  // ASSERTIONS:
  // - *dpinfop valid

  return pagep;
}


//********************************************************************

// Insert a record into the file

Status HeapFile::insertRecord(char* recPtr, int recLen, RID& outRid)
{
  struct DataPageInfo dpinfo;
  int dpinfoLen;

  Status st; // temporary variable
  Status status;
  bool found;

  PageId actualFirstDirPageId =  _firstDirPageId;
  PageId currentDirPageId = _firstDirPageId;
  HFPage* currentDirPage;
  PageId nextDirPageId;
  HFPage* nextDirPage;

  // currentDataPageId is stored in dpinfo.pageId
  HFPage *currentDataPage = NULL;
  RID currentDataPageRid;

  // user record id is in outRid



//  if (recLen > (DATASIZE-DPFIXED)) 
//     return MINIBASE_CHAIN_ERROR( HEAPFILE, DBMGR );

/* For insert, lock all pages touched in the 'Exclusive' mode */


  // get the first directory page (initialized to _firstDirPageId )
  st = MINIBASE_BM->pinPage(currentDirPageId, (Page *&) currentDirPage);
  if (st != OK)
      return MINIBASE_CHAIN_ERROR( HEAPFILE, st );
  
  // ASSERTOINS:
  // currentDirPageId, currentDirPage valid and pinned

  found = false;
  while (found == false)
  {
    
    // look for suitable dpinfo-struct
    for (status = currentDirPage->firstRecord(currentDataPageRid);
	 status == OK;
	 status = currentDirPage->nextRecord(currentDataPageRid,
					     currentDataPageRid)) {

      st = currentDirPage->getRecord(currentDataPageRid,
				     (char *) &dpinfo, dpinfoLen);
      if (st != OK)
	  return MINIBASE_CHAIN_ERROR( HEAPFILE, st );

      if (dpinfoLen != sizeof(DataPageInfo))
	  return MINIBASE_CHAIN_ERROR( HEAPFILE, DBMGR );

      if (recLen <= dpinfo.availspace) { 
	found = true;
	break;
      }
    }

    // two cases:
    // (1) found == true:
    //     currentDirPage has a datapagerecord which can accomodate
    //     the record which we have to insert
    // (2) found == false:
    //     there is no datapagerecord on the current directory page
    //     whose corresponding datapage has enough space free
    //     several subcases: see below

    if (found == false)
    {
      // case (2)
      // on the current directory page is no datapagerecord which has
      // enough free space
      //
      // two cases:
      //
      // - (2.1) (currentDirPage->available_space() >= sizeof(DataPageInfo):
      //         if there is enough space on the current directory page
      //         to accomodate a new datapagerecord (type DataPageInfo),
      //         then insert a new DataPageInfo on the current directory
      //         page
      // - (2.2) (currentDirPage->available_space() <= sizeof(DataPageInfo):
      //         look at the next directory page, if necessary, create it.

      if (currentDirPage->available_space() >= sizeof(DataPageInfo)) {

	// case (2.1) : add a new data page record into the
	//              current directory page

	currentDataPage = _newDatapage(&dpinfo); 
	// currentDataPage is pinned! and dpinfo->pageId is also locked
	// in the exclusive mode

	if (currentDataPage == NULL)
	    return MINIBASE_CHAIN_ERROR( HEAPFILE, DBMGR );
      
	// currentDataPage is pinned: insert its record
	/* calling a HFPage function */
	st = currentDirPage->insertRecord((char *)&dpinfo,
					  sizeof(dpinfo), currentDataPageRid);
	if (st != OK)
	    return MINIBASE_CHAIN_ERROR( HEAPFILE, st );

	// end the loop, because a new datapage with its record
	// in the current directorypage was created and inserted into
	// the heapfile; the new datapage has enough space for the
	// record which the user wants to insert
	found = true;
      }
      else
      {
	// case (2.2)

      
	nextDirPageId = currentDirPage->getNextPage();

	// two sub-cases:
	//
	// (2.2.1) nextDirPageId != INVALID_PAGE:
	//         get the next directory page from the buffer manager
	//         and do another look
	// (2.2.2) nextDirPageId == INVALID_PAGE:
	//         append a new directory page at the end of the current
	//         page and then do another loop

	if (nextDirPageId != INVALID_PAGE)
	{
	  // case (2.1): there is another directory page:

	  st = MINIBASE_BM->unpinPage(currentDirPageId, FALSE);
	  
	  if (st != OK)
	      return MINIBASE_CHAIN_ERROR( HEAPFILE, st );
	  
	  currentDirPageId = nextDirPageId; 
	  st = MINIBASE_BM->pinPage(currentDirPageId, (Page*& )currentDirPage);
	  if (st != OK)
	      return MINIBASE_CHAIN_ERROR( HEAPFILE, st );

	  // now go back to the beginning of the outer while-loop and
	  // search on the current directory page for a suitable datapage
	}
	else
	{
	  // case (2.2): append a new directory page after currentDirPage
	  //             since it is the last directory page
	  st = MINIBASE_BM->newPage(nextDirPageId, (Page *&) nextDirPage);
	  
	  if (st != OK)
	      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
	  
	  if ( nextDirPage == NULL)
	      return MINIBASE_CHAIN_ERROR( HEAPFILE, DBMGR );
	    
	  // initialize new directory page

	  nextDirPage->init(nextDirPageId);
	  nextDirPage->setNextPage(INVALID_PAGE);
	  nextDirPage->setPrevPage(currentDirPageId);
	  
	  // update current directory page and unpin it
	  // currentDirPage is already locked in the Exclusive mode

	  currentDirPage->setNextPage(nextDirPageId);
	  st = MINIBASE_BM->unpinPage(currentDirPageId, TRUE);
	  if (st != OK)
	      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
	  
	  currentDirPageId = nextDirPageId;
	  currentDirPage = nextDirPage;

	  // remark that MINIBASE_BM->newPage already pinned the new directory page!
	  // now back to the beginning of the while-loop, using the
	  // newly created directory page
	}
      }
      
      // ASSERTIONS:
      // - if found == true: search will end and see assertions below
      // - if found == false: currentDirPage, currentDirPageId valid and pinned
 
    }
    else
    {
      // found == true:
      // we have found a datapage with enough space,
      // but we have not yet pinned the datapage:

      // ASSERTIONS:
      // - dpinfo valid

      st = MINIBASE_BM->pinPage(dpinfo.pageId, (Page*& )currentDataPage);
      if (st != OK)
	  return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
    }
  }

  // ASSERTIONS:
  // - currentDirPageId, currentDirPage valid and pinned
  // - dpinfo.pageId, currentDataPageRid valid
  // - currentDataPage is pinned!


  if (dpinfo.pageId == INVALID_PAGE)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, DBMGR );


  if (!(currentDataPage->available_space() >= recLen))
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
  
  if (currentDataPage == NULL)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

  st = currentDataPage->insertRecord(recPtr, recLen, outRid);
  if (st != OK)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

  dpinfo.recct++;
  dpinfo.availspace = currentDataPage->available_space();


  st = MINIBASE_BM->unpinPage(dpinfo.pageId, TRUE /* = DIRTY */);
  if (st != OK)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

  // DataPage is now released

  DataPageInfo *pdpinfo_ondirpage;
  st = currentDirPage->returnRecord(currentDataPageRid,
				    (char *&) pdpinfo_ondirpage, dpinfoLen);
  if (st != OK)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

  if (dpinfoLen != sizeof(DataPageInfo))
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, DBMGR );
  // Need to log this update to the CurrentDirPage, so that the dpInfo stays consistent.
  *pdpinfo_ondirpage = dpinfo;  // actually modify the dir page


  st = MINIBASE_BM->unpinPage(currentDirPageId, TRUE /* = DIRTY */);
  if (st != OK)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
  
  return OK;
} /* a bit confusing... Ranjani */



// -----------------------------------------------------------------------
/*
 * bool HeapFile::_findDataPage (const RID& rid,
 *			     PageId& rpDirPageId, HFPage*& rpdirpage,
 *			     PageId& rpDataPageId, HFPage*& rpdatapage,
 *			     RID& rpDataPageRid)
 *
 * Internal HeapFile function (used in getRecord and updateRecord): returns
 * pinned directory page and pinned data page of the specified user record
 * (rid) and true if record is found.
 *
 * If the user record cannot be found, rpdirpage and rpdatapage are 
 * returned as NULL pointers and we return false.
 */

Status HeapFile::_findDataPage (const RID& rid,
			     PageId& rpDirPageId, HFPage*& rpdirpage,
			     PageId& rpDataPageId, HFPage*& rpdatapage,
			     RID& rpDataPageRid)
{
    DataPageInfo dpinfo;
    int dpinfoLen;
    
    Status status; 
    Status st;
    
    PageId currentDirPageId = _firstDirPageId;
    HFPage* currentDirPage;
    RID currentDataPageRid;
    // datapageId is stored in dpinfo.pageId 
    HFPage* currentDataPage;
    
    PageId nextDirPageId;
    
    st = MINIBASE_BM->pinPage(currentDirPageId, (Page*& ) currentDirPage);
    if (st != OK)
	return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );


    // better have first dir page in core!
    
    while (currentDirPageId != INVALID_PAGE)
    {

	// ASSERTIONS:
	// - currentDirPage, currentDirPageId valid and pinned and Locked.
	
	for (status = currentDirPage->firstRecord(currentDataPageRid);
	     status == OK;
	     status = currentDirPage->nextRecord(currentDataPageRid,
						 currentDataPageRid)) {
	    
	    st = currentDirPage->getRecord(currentDataPageRid,
					   (char *) &dpinfo, dpinfoLen);
	    if (st != OK)
		return DONE;  // NOT really an error if we can't find it!
	    
	    if (dpinfoLen != sizeof(DataPageInfo))
		return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
	    
	    st = MINIBASE_BM->pinPage(dpinfo.pageId, (Page *&) currentDataPage);
	    if (st != OK) {  // no mem
		MINIBASE_BM->unpinPage(currentDirPageId);
		rpdirpage = rpdatapage = NULL;
		return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
	    }
  
	    // ASSERTIONS:
	    // - currentDataPage, currentDataPageRid, dpinfo valid
	    // - currentDataPage pinned
	    
	    char* dummyrecptr;
	    int dummyreclen;

	    // now here is a bug: the functions getRecord and returnRecord of
	    //   the class HFPage only compare the slots of the RIDs. This is
	    // a bad
	    //   design decision, since we now have here to know something
	    //   about the internal structure of RID
	    // check this out:
	    // typedef struct RID{
	    // PageId  pageNo;
	    // int  slotNo;
	    // int operator==(const RID rid) const
	    //   {return pageNo==rid.pageNo&&slotNo==rid.slotNo;};
	    // };
	    // so returnRecord does not even use == to compare RIDs!

	    if ((dpinfo.pageId == rid.pageNo) &&
		currentDataPage->returnRecord(rid,
					      dummyrecptr, 
					      dummyreclen) == OK) {
		
		// found user's record on the current datapage which itself is
		// indexed on the current dirpage.  Return both of these.

		rpdirpage = currentDirPage;
		rpDirPageId = currentDirPageId;
		rpdatapage = currentDataPage;
		rpDataPageId = dpinfo.pageId;
		rpDataPageRid = currentDataPageRid; 
		return OK;
	    }
	    else {
		// user record not found on this datapage; unpin it
		// and try the next one
		st = MINIBASE_BM->unpinPage(dpinfo.pageId);
	       
		if (st != OK)
		    return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
	    }
	}
	
	// if we would have found the correct datapage on the current
	// directory page we would have already returned.
	// therefore:
	// read in next directory page:
	
	nextDirPageId = currentDirPage->getNextPage();
	st = MINIBASE_BM->unpinPage(currentDirPageId);
	if (st != OK)
	    return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
	
    
	currentDirPageId = nextDirPageId;
	if (currentDirPageId != INVALID_PAGE)
	{
	    st =MINIBASE_BM->pinPage(currentDirPageId, (Page*&) currentDirPage);
	    if (st == OK)
		return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
	    if (currentDirPage == NULL)
		return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
        }    
  }
  
  /* checked all dir pages and all data pages; user record not found */
  rpdirpage = rpdatapage = NULL;
  rpDirPageId = rpDataPageId = INVALID_PAGE;
  return DONE;
}


// -----------------------------------------------------------------------
// read record from file, returning pointer and length
Status HeapFile::getRecord (const RID& rid, char* const recPtr, int& recLen)
{
  Status st;

  HFPage *dirpage;
  PageId currentDirPageId;
  HFPage *datapage = NULL;
  PageId currentDataPageId;
  RID    currentDataPageRid;

  Status foundrec = _findDataPage(rid,
			       currentDirPageId, dirpage,
			       currentDataPageId, datapage,
			       currentDataPageRid);
  
  if ( foundrec != OK)
      return foundrec;


  st = datapage->getRecord(rid, recPtr, recLen);
  if (st != OK)  // otherwise _findDataPage (called above) is broken.
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
  

  /*
   * getRecord has copied the contents of rid into recPtr and fixed up
   * recLen also.  We simply have to unpin dirpage and datapage which
   * were originally pinned by _findDataPage.
   */
  
  st = MINIBASE_BM->unpinPage(currentDataPageId);
  if (st != OK)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

  st = MINIBASE_BM->unpinPage(currentDirPageId);
  if (st != OK)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

  return OK;
}


// -----------------------------------------------------------------------
// delete record from file
Status HeapFile::deleteRecord (const RID& rid)
{
  Status st;

  HFPage* currentDirPage;
  PageId currentDirPageId;
  HFPage* currentDataPage = NULL;
  PageId currentDataPageId;
  RID    currentDataPageRid;
  HFPage* prevDirPage;
  

  DataPageInfo *pdpinfo;
  int dpinfolen; 
  
  Status foundrec = _findDataPage(rid,
			       currentDirPageId, currentDirPage, 
			       currentDataPageId, currentDataPage,
			       currentDataPageRid);
  if ( foundrec != OK)
      return foundrec;


  // ASSERTIONS:
  // - currentDirPage, currentDirPageId valid and pinned
  // - currentDataPage, currentDataPageid valid and pinned


  // get datapageinfo from the current directory page:
  st = currentDirPage->returnRecord(currentDataPageRid,
			     (char *&) pdpinfo , dpinfolen);
  if (st != OK)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
  if (dpinfolen != sizeof(DataPageInfo))
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
  if (pdpinfo->pageId != currentDataPageId)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );


  // delete the record on the datapage
  st = currentDataPage->deleteRecord(rid);
  if (st != OK)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
  pdpinfo->recct--;

  if (pdpinfo->recct >= 1) {
      // more records remain on datapage so it still hangs around.  
      // we just need to modify its directory entry
      pdpinfo->availspace = currentDataPage->available_space();
      st = MINIBASE_BM->unpinPage(currentDataPageId, TRUE /* = DIRTY*/);
      if (st != OK)
	  return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
      st = MINIBASE_BM->unpinPage(currentDirPageId, TRUE /* = DIRTY */);
      if (st != OK)
	  return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
  }
  else {
     // the record is already deleted:
    // we're removing the last record on datapage so free datapage
    // also, free the directory page if 
    //   a) it's not the first directory page, and 
    //   b) we've removed the last DataPageInfo record on it.

    // delete empty datapage: (does it get unpinned automatically? -NO, Ranjani)
    st = MINIBASE_BM->unpinPage(currentDataPageId);
    if (st != OK)
	return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

    st = MINIBASE_BM->freePage(currentDataPageId);
    if (st != OK)
	return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );


    // delete corresponding DataPageInfo-entry on the directory page:
    // currentDataPageRid points to datapage (from for loop above)
    st = currentDirPage->deleteRecord(currentDataPageRid);
    if (st != OK)
	return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

    // ASSERTIONS:
    // - currentDataPage, currentDataPageId invalid
    // - empty datapage unpinned and deleted

    // now check whether the directory page is empty:
    st = currentDirPage->firstRecord(currentDataPageRid);

    // st == OK: we still found a datapageinfo record on this directory page
    if ((st != OK) && (currentDirPage->getPrevPage()!=INVALID_PAGE))
    {
      // the directory-page is not the first directory page and it is empty:
      // delete it

      // point previous page around deleted page:
      st = MINIBASE_BM->pinPage(currentDirPage->getPrevPage(), (Page*& )prevDirPage);
      if (st != OK)
	  return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );


      prevDirPage->setNextPage(currentDirPage->getNextPage());
      st = MINIBASE_BM->unpinPage(currentDirPage->getPrevPage(), 
				  TRUE /* = DIRTY */);
      if (st != OK)
	  return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

      // set prevPage-pointer of next Page:
      if (currentDirPage->getNextPage() != INVALID_PAGE)
      {
	HFPage *nextDirPage;

	st = MINIBASE_BM->pinPage(currentDirPage->getNextPage(), (Page*& )nextDirPage);
	if (st != OK)
	    return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

	nextDirPage->setPrevPage(currentDirPage->getPrevPage());
	st = MINIBASE_BM->unpinPage(currentDirPage->getNextPage(), TRUE /* = DIRTY */);
	if (st != OK)
	    return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

      }
      
    
      // delete empty directory page: (automatically unpinned?)
      st = MINIBASE_BM->unpinPage(currentDirPageId);
      if (st != OK)
	  return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );  
 
      st = MINIBASE_BM->freePage(currentDirPageId);
      if (st != OK)
	  return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );   
    }
    else
    {
      // either (the directory page has at least one more datapagerecord
      // entry) or (it is the first directory page):
      // in both cases we do not delete it, but we have to unpin it:

      st = MINIBASE_BM->unpinPage(currentDirPageId, TRUE /* == DIRTY */);
      if (st != OK)
	  return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );   
    }    
  }
  
  return OK;
}

// -----------------------------------------------------------------------
// updates the specified record in the heapfile.
Status HeapFile::updateRecord (const RID& rid, char* recPtr, int recLen)
{
  Status st;

  HFPage* dirpage;
  PageId currentDirPageId;
  HFPage* datapage;
  PageId currentDataPageId;
  RID    currentDataPageRid;
  
  
  Status foundrec = _findDataPage(rid,
			       currentDirPageId, dirpage,
			       currentDataPageId, datapage,
			       currentDataPageRid);
 
  if (foundrec != OK)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, DBMGR );


  char *oldrecptr;
  int oldreclen;
  
  st = datapage->returnRecord(rid, oldrecptr, oldreclen);
  if (st != OK)  // otherwise _findDataPage (called above) is broken.
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );


  if (recLen != oldreclen) {
    // they tried to update a record with data different in size from the old
    // record; that's an error since we can't necessarily expand the
    // old record (to accommodate the extra bytes of the new record), and
    // we can't pick a new slot/new page for the new record because that
    // would change the RID: but we can't change the RID because it's const&.

        // There is no good reason to prohibit updates that shorten a record,
        // but we do that anyway.
    st = MINIBASE_BM->unpinPage(currentDataPageId);
    if (st != OK)
	return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

    st = MINIBASE_BM->unpinPage(currentDirPageId);
    if (st != OK)
	return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
    
    return MINIBASE_FIRST_ERROR( HEAPFILE, INVALIDUPDATE );
  }
    
  // new copy of this record fits in old space; easy.
 
  /* Now the hairy part of logging - instead of logging exactly which
  record that is being modified, we just log the before and after
  images of the page - as it is a huge complicated mess otherwise */



  memcpy(oldrecptr, recPtr, recLen); /* the page has been modified
				     and is locked  & logged */

  st = MINIBASE_BM->unpinPage(currentDataPageId, TRUE /* = DIRTY */);
  if (st != OK)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

  st = MINIBASE_BM->unpinPage(currentDirPageId);
  if (st != OK)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

  return OK;
}


// initiate a sequential scan
Scan* HeapFile::openScan (Status& status)
{
  // sure we need:
  Scan* newScan;

  newScan = new Scan(this, status);

  if (status == OK)
    return newScan;
  else
  {
    delete newScan;
    return NULL;
  }
}
