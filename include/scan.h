/* -*- C++ -*- */
/* 
 * scan.h -  class Scan
 *
 * Johannes Gehrke & Gideon Glass  950920  CS564  UW-Madison
 *
 * modified by michael lee 1995
 */

#ifndef _SCAN_H_
#define _SCAN_H_


#include "minirel.h"


// ***********************************************************
// A Scan object is created ONLY through the function openScan
// of a HeapFile. It supports the getNext interface which will 
// simply retrieve the next record in the heapfile.
//
// an object of type scan will always have pinned one directory page
//   of the heapdile

class HeapFile;
class HFPage;

class Scan
{
public:

  // The constructor pins the first directory page in the file
  // and initializes its private data members from the private
  // data member from hf
  Scan(HeapFile* hf, Status& status);

  //
  ~Scan();

  // retrieve the next record in a sequential scan
  // Also returns the RID of the retrieved record.
  Status getNext(RID& rid, char* recPtr, int& recLen );

  

private:

/*
 * See heapfile.h for the overall description of a heapfile.
 * (Then see hfpage.h for HFPage ops.)
 *
 * Note that one record in our way-cool HeapFile implementation is
 * specified by six (6) parameters, some of which can be determined
 * from others:
 */

  PageId dirpageId;
  // PageId of current directory page (which is itself an HFPage)
  HFPage *dirpage;
  // pointer to in-core data of dirpageId (page is pinned)

  RID datapageRid;
  // record ID of the DataPageInfo struct (in the directory page) which
  // describes the data page where our current record lives.
  PageId datapageId;
  // the actual PageId of the data page with the current record
  HFPage *datapage;
  // in-core copy (pinned) of the same
    
  RID userrid;
  // record ID of the current record (from the current data page)


};

#endif
