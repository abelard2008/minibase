
#ifndef ERROR_H
#define ERROR_H

// error/status codes defined here

// add error codes under appropriate heading
// -- remember to add corresponding message to
//    Error.print() also!

enum Status {

// no error

       OK = 0, NOTUSED1 = -999,

// Tuple errors

	  TUPLE_TOO_BIG,
	  TUPLE_TYPE,

// File and DB errors

       BADFILEPTR, BADFILE, FILETABFULL, FILEOPEN,
       UNIXERR, BADPAGEPTR, BADPAGENO, FILEEXISTS,

// BufMgr and HashTable errors

       HASHTBLERROR, HASHNOTFOUND, BUFFEREXCEEDED, PAGENOTPINNED,
       BADBUFFER, PAGEPINNED,

// Page errors
	
       NOSPACE,  NORECORDS,  ENDOFPAGE, INVALIDSLOTNO,

// HeapFile errors

       BADRID, BADRECPTR, BADSCANPARM, BADSCANID, SCANTABFULL, FILEEOF,

// Index errors
 
       BADINDEXPARM, RECNOTFOUND, BUCKETFULL, DIROVERFLOW, 
       NONUNIQUEENTRY, NOMORERECS,

// SortedFile errors
 
       BADSORTPARM, INSUFMEM, 
	
// Catalog errors

       BADCATPARM, RELNOTFOUND, ATTRNOTFOUND,
       NAMETOOLONG, DUPLATTR, RELEXISTS, NOINDEX,
       INDEXEXISTS, ATTRTOOLONG,

// Utility errors

// Query errors

// do not touch filler -- add codes before it

       NOTUSED2
};


// definition of Error class

class Error {
 public:
  void print(Status status);
};

#endif
