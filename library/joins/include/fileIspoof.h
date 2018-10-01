#ifndef __FILEISPOOF__
#define __FILEISPOOF__

//#include "test.h"
#include "tuple.h"

#include "heapfile.h"
#include "scan.h"           // added by BK

class spoofI_buf {
 public:
   spoofI_buf();
   ~spoofI_buf();
#ifdef TEST
   void init(int fd, char **bufs, int n_pages, int tSize, int n_tuples);
#else
   void init(HeapFile* fd, char **bufs, int n_pages, int tSize,
	     int n_tuples, Status &s);
#endif
   Status Get(Tuple * & buf);
   int empty(void);
 private:
   int readin(void);
   
   char **_bufs;
#ifdef TEST
   int    _fd;
#else
   HeapFile *_fd;
   Scan *hf_scan;
#endif
   int    _n_pages;
   int    t_size;
   
   int    t_proc, t_in_buf;
   int    tot_t_proc;
   int    t_rd_from_pg, curr_page;
   int    t_per_pg;
   int   done;
   char  *dummy;
   Tuple *op_tuple;
   int    n_tuples;
};

#ifdef TEST
char * temp_file_name(void);
#endif

#endif
