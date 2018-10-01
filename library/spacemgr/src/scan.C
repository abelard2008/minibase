/*
 * scan.C - implementation of class Scan for HeapFile project.
 *
 * Johannes Gehrke & Gideon Glass  950920  CS564  UW-Madison
 *
 * modified by Michael Lee, 1995
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "heapfile.h"
#include "scan.h"
#include "hfpage.h"
#include "buf.h"
#include "db.h"

Scan::Scan (HeapFile* hf, Status& status)
{
    	     
  DataPageInfo dpinfo;
  int dpinfoLen;
  Status st;

  // copy data about first directory page
  dirpageId = hf->_firstDirPageId;  


  // get first direcroty page and pin it
    st = MINIBASE_BM->pinPage(dirpageId, (Page *&) dirpage);
    if (st != OK) 
    {
	status = st;
	MINIBASE_CHAIN_ERROR( HEAPFILE, st);
	return;
    }
	
  // now try to get a pointer to the first datapage
  // and set vars for scan::next
  st = dirpage->firstRecord(datapageRid);
  if (st == OK)
  {
    // there is a datapage record on the first directory page:
    st = dirpage->getRecord(datapageRid, (char *)&dpinfo, dpinfoLen);
    if (!(st==OK))
    {
	status = st;
	MINIBASE_CHAIN_ERROR( HEAPFILE, st);
	return;	
    }
    
    if (!(dpinfoLen == sizeof(DataPageInfo)))
    {
	status = FAIL;
	MINIBASE_CHAIN_ERROR( HEAPFILE, st);
	return;	
    }
    
    datapageId = dpinfo.pageId;
  }
  else
  {
    // the first directory page is the only one which can possibly remain
    // empty: therefore try to get the next directory page and
    // check it. The next one has to contain a datapage record, unless
    // the heapfile is empty:

    PageId nextDirPageId;
    
    nextDirPageId = dirpage->getNextPage();
    if (nextDirPageId != INVALID_PAGE)
    {
      st = MINIBASE_BM->unpinPage(dirpageId, FALSE);
      dirpage = 0;
      if (st != OK)
      {
	  status = st;
	  MINIBASE_CHAIN_ERROR( HEAPFILE, st);
	  return;
      }


      st = MINIBASE_BM->pinPage(nextDirPageId, (Page*& )dirpage);
      if (st != OK)
      {
	  status = st;
	  MINIBASE_CHAIN_ERROR( HEAPFILE, st);
	  return;
      }
      
      
      // now try again to read a data record:
      st = dirpage->firstRecord(datapageRid);
      if (st == OK)
      {
	st = dirpage->getRecord(datapageRid, (char *)&dpinfo, dpinfoLen);
	if (st != OK)
	{
	    status = st;
	    MINIBASE_CHAIN_ERROR( HEAPFILE, st);
	    return;	    
	}
	
	if (dpinfoLen != sizeof(DataPageInfo))
	{
	    status = st;
	    MINIBASE_CHAIN_ERROR( HEAPFILE, st);
	    return;	    
	}
	
	datapageId = dpinfo.pageId;
      }
      else
      {
	// heapfile empty
	// set vars for scan::next
	datapageId = INVALID_PAGE;
      }
    }
    else
    {
      // heapfile empty
      // set vars for scan::next
      datapageId = INVALID_PAGE;
    }    
  }

  status = OK;
  datapage = NULL;
  

  // ASSERTIONS:
  // - first directory page pinned
  // - this->dirpageId has Id of first directory page
  // - this->dirpage valid
  // - if heapfile empty:
  //	- this->datapage == NULL, this->datapageId==INVALID_PAGE
  // - if heapfile nonempty:
  //	- this->datapage == NULL, this->datapageId, this->datapageRid valid
  //    - first datapage is not yet pinned
}

Scan::~Scan ()
{
  Status st;

  if (datapage != NULL)
  {
    st = MINIBASE_BM->unpinPage(datapageId);
    if (st != OK)
	MINIBASE_CHAIN_ERROR( HEAPFILE, st);
  }
  if (dirpage != NULL)
  {
    st = MINIBASE_BM->unpinPage(dirpageId);
    if (st != OK)
	MINIBASE_CHAIN_ERROR( HEAPFILE, st);
  }
}

// retrieve the next record in a sequential scan
// Also returns the RID of the retrieved record.
Status Scan::getNext(RID& rid, char* recPtr, int& recLen)
{
  DataPageInfo dpinfo;
  int dpinfoLen;

  Status st;
  Status nextUserRecordStatus;
  Status nextDataPageStatus;

  PageId nextDirPageId;
  

  // ASSERTIONS:
  // - this->dirpageId has Id of current directory page
  // - this->dirpage is valid and pinned
  // (1) if heapfile empty:
  //	- this->datapage==NULL; this->datapageId == INVALID_PAGE
  // (2) if overall first record in heapfile:
  //    - this->datapage==NULL, but this->datapageId valid
  //	-.this->datapageRid valid
  //    - current data page unpinned !!!
  // (3) if somewhere in heapfile
  //	- this->datapageId, this->datapage, this->datapageRid valid
  //	- current data page pinned

  if (datapage == NULL)
  {
    if (datapageId == INVALID_PAGE)
    {
      // heapfile is empty to begin with
      st = MINIBASE_BM->unpinPage(dirpageId);
      dirpage = NULL;
      if (st != OK)
	  return MINIBASE_CHAIN_ERROR( HEAPFILE, st );
      return DONE;
    }
    else
    {
      // read very first data record of the heapfile
      // and return

      // pin first data page

      st = MINIBASE_BM->pinPage(datapageId, (Page *&) datapage);
      if (st != OK)
	  return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
            
      // read the record
      st=datapage->firstRecord(userrid);
      if (st != OK)
	  return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
      
      rid = userrid;
      st = datapage->getRecord(rid, recPtr, recLen);
      if (st != OK)
	  return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

      return OK;
    }
    
  }
  
  // ASSERTIONS:
  // - this->datapage, this->datapageId, this->datapageRid valid
  // - current datapage pinned

  nextUserRecordStatus = datapage->nextRecord( userrid, rid);

  if (nextUserRecordStatus == OK)
  {
    userrid = rid; // save rid for next call to Heapfile::next()
    st = datapage->getRecord (rid, recPtr, recLen);
    if (st != OK)
	return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
    return st;
  }
  
  // ASSERTIONS:
  // - nextUserRecordStatus != OK and the error is that there are no
  //	more records on that datapage

  // we have read all records on the current datapage: unpin it
  st=MINIBASE_BM->unpinPage(datapageId);
  datapage = NULL;
  if (st != OK)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
  
  // read next datapagerecord from current directory page
  nextDataPageStatus = dirpage->nextRecord(datapageRid, datapageRid);

  if (nextDataPageStatus !=OK)
  {    
    // we have read all datapage records on the current directory page

    // get next directory page
    nextDirPageId = dirpage->getNextPage();

    // unpin the current directory page
    st=MINIBASE_BM->unpinPage(dirpageId);
    dirpage = NULL;
    if (st != OK)
	return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

    if (nextDirPageId == INVALID_PAGE)
      return DONE;
    else
    {
      // ASSERTION:
      // - nextDirPageId has correct id of the page which is to get

      dirpageId = nextDirPageId;

      Status st = MINIBASE_BM->pinPage(dirpageId, (Page *&) dirpage);
      if (st != OK)
	  return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
      
      if (dirpage == NULL)
	  return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

      nextDataPageStatus = dirpage->firstRecord(datapageRid);
    }
    
  }
  
  // ASSERTION:
  // - this->dirpageId, this->dirpage valid
  // - this->dirpage pinned
  // - the new datapage to be read is on dirpage
  // - this->datapageRid has the Rid of the next datapage to be read
  // - this->datapage, this->datapageId invalid

  // data page is not yet loaded: read its record from the directory page
  st = dirpage->getRecord(datapageRid, (char *) &dpinfo, dpinfoLen);
  if (st != OK)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
  if (dpinfoLen != sizeof(DataPageInfo))
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

  // now get and pin the datapage
  datapageId = dpinfo.pageId;


  st = MINIBASE_BM->pinPage(dpinfo.pageId, (Page *&) datapage);
  if (st!=OK)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );
  
  // - first directory page is pinned
  // - first datapage is pinned
  // - this->dirpageId, this->dirpage correct
  // - this->datapageId, this->datapage, this->datapageRid correct
  
  nextUserRecordStatus = datapage->firstRecord(rid);
  if (nextUserRecordStatus != OK)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

  // this datapage is not empty
  userrid = rid; // save rid for next call of Heapfile::next()

  st = datapage->getRecord (rid, recPtr, recLen);
  if (st!=OK)
      return  MINIBASE_CHAIN_ERROR( HEAPFILE, st );

  return OK;
}
