// linearhashing in MINIREL
// Weiqing Huang, whuang@cs.wisc.edu, May 1995

// $Id: linearhashing.h,v 1.5 1996/07/17 19:39:34 ramamurt Exp $

#ifndef _LINEARHASHING_H_
#define _LINEARHASHING_H_



#include <values.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "lh_error.h"
#include "buf.h"
#include "index.h"
#include "new_error.h"

class LinearHashingFileScan;		// Forward declaration.

class LinearHashingFile : public IndexFile {
private:
    friend class LinearHashingFileScan;

    // the num of directories (page no) on one page
    const int NUM_DIR_PER_PAGE = MAX_SPACE / sizeof(PageId);

    // the number PRIME is used in hashing function.
    // in order to have the hashing function work well, the number of
    // buckets should be less than PRIME.
    const long PRIME = 99999971;
    
    // dir page will only contain pageno's
    typedef PageId Dir;

    // page to contain records
    class Bucket { 
    public:
        short RecCnt;           // the count of the records in the bucket
        PageId OverflowPageNo;  // the overflow page's no, if no overflowpage, no = -1
        char data[MAX_SPACE - sizeof(short) -sizeof(PageId)]; // the data area 
    };

    // root page
    class LHRootPage {
    public:
        PageId        HeaderPageNo;
    };
    
    // header page
    class LHHeaderPage {
    public:
        char          FileName[MAXINDEXNAME]; // name of the index
        int           length;                 // length of the attribute
        AttrType      type;                   // AttrType of the attribute
        int           level;                  // hashing function level
        int           next;                   // the next bucket to split
        int           N;                      // num of initial buckets
        PageId        first_dir_pageno;       // num of the first dir page
        int           NumDirPage;             // num of dir pages

    };


    LHRootPage *  RootPage;      // the root page
    LHHeaderPage* HeaderPage;    // the header page  

private:
    PageId        RootPageNo;    // the pageno of the root page
    PageId        HeaderPageNo;  // the pageno of the header page
    int           NumSlots;      // maximum number of slots in a page
    int           RecSize;       // record size
    int           destroyed;     // has the index been destroyed?
    uint          SpaceForName;  // the real space (in bytes) the
                                 // HeaderPage->FileName occupies. It's
                                 // used when writing updatelog.
                                 // due to alignment, it could be greater
                                 // than MAXINDEXNAME. 

    // private functions:
    
    // Test if entry "value, rid" is in the "slotnum" of 'bucket'
    Status MatchRec(const Bucket* bucket, const void *value, const RID rid, const int slotnum) const ;

    // Test if entry "value" is in the "slotnum" of 'bucket'
    Status MatchKey(const Bucket* bucket, const void *value, const int slotnum) const ;

    // hashing function h: value --> hashvalue
    Status HashIndex(const void *value, int& hashvalue) const ;

    // merge operation
    Status merge();

    // get the pageno of bucket "num"
    Status get_dir (const int num, PageId & pageno) const;
    
    // set the pageno of bucket "num"
    Status set_dir (const int num, const PageId pageno);

    // delete the last bucket's pageno in the dir
    Status del_last_dir (void);

    // num of current buckets 
    int num_buckets() const {
        return ((int)pow(2,HeaderPage->level))*HeaderPage->N+HeaderPage->next;}

    // methods to set values of next, level, a bucket's RecCnt and a
    // bucket's OverflowPageNo 
    // they are used to facilitate the writing of update logs
    // (if no log needs to be written, one can just use bucket->RecCnt++
    // etc. to update the field.) 
    Status set_next(const int new_next);
    Status set_level(const int new_level);
    Status set_RecCnt(Bucket* bucket, const PageId pageno, const int new_RecCnt, const int new_bucket = TRUE);
    Status set_OverflowPageNo(Bucket* bucket, const PageId pageno, const PageId overflowpageno,const int new_bucket = TRUE);

    Status _insert(const void* value, const RID rid);
    Status _Delete(const void* value, const RID rid);

public:
    friend class LinearHashingScan;
    
    // constructor for the LinearHashingFile class -- create the index
    // name -- name of the index
    // length, type -- describing the key attribute
    // N -- the number of initial buckets, default = 1
    LinearHashingFile(Status& status, const char* name, const int length,
                      const AttrType type, const int N = 1); 

    // constructor for the LinearHashingFile class -- open the index
    // assume the index already exists
    // name -- name of the index
    LinearHashingFile(Status& status, const char * name);

    // destructor
    ~LinearHashingFile();

    // destroy the index
    Status destroyFile(void);
    
    // insert an entry <value, rid> to the index
    Status insert(const void* value, const RID rid);

    // delete an entry <value, rid> from the index
    Status Delete(const void* value, const RID rid);
    
    // open an exact match scan with "value" as the key
    IndexFileScan * new_scan(const void* value);
    IndexFileScan * new_scan();


    int keysize() { return HeaderPage->length; }

    void PrintHeader();             // print the header info
    void PrintBucket();             // print all the buckets
    void PrintBucket(int BucNum);   // print bucket "BucNum"
};


class LinearHashingScan : public IndexFileScan {
private:
    typedef LinearHashingFile::Bucket Bucket;
    
    LinearHashingFile * LHFile; // the file to be scanned
    void * key;                 // the key to scan 
    int index;                  // the hashvalue of the key
    PageId current_pageno;      // the no of the page containing the currently scanned record
    int current_slotno;         // the slotno of the currently scanned record
    Bucket * current_bucket;    // the bucket containing the currently scanned record
    int first_scan;             // has the scan started?
    PageId prev_pageno;         // the no of the previous scanned page
    PageId last_pageno;         // the no of the last page to scan.
                                // see merge_in_scan() for reason
    int read;                   // any record read from currentpage?
                                // used for releasing read lock

    // private functions:
    
    // constructor for LinearHashingScan
    LinearHashingScan(LinearHashingFile* lhf, const void * value);

    // merge buckets during scan
    Status merge_in_scan();

    // this merge is different from LinearHashingFile::merge()
    // because it will affect the currently scanned bucket
    Status _merge_in_scan();

    Status _delete_current();
    
public:
    friend class LinearHashingFile;
    
    // get the next record 
    Status get_next(RID & rid, void* keyptr);

    // delete the record currently scanned
    Status delete_current();

    int keysize() { return LHFile->keysize(); }

    // destructor
    ~LinearHashingScan();

};
    
    
class LinearHashingFileScan : public IndexFileScan {
 private:
   int bkt_no;
   PageId PageNo;
   LinearHashingFile::Bucket * pg;
   LinearHashingFile * _plh;
   int i;

 public:
   LinearHashingFileScan(LinearHashingFile * plh, Status &s);
   Status get_next(RID &rid, void* keyptr);
   ~LinearHashingFileScan();
   Status delete_current(){ 
       return FAIL;
   }
   int keysize(){
       return _plh->keysize();
   }
};
#endif

