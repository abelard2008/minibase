#ifndef __IO_BUFS__
#define __IO_BUFS__

#include "tuple.h"
#include "fileIspoof.h"

class O_buf {
 public:
   O_buf();				// Constructor -- no args -- use init to initialize
   ~O_buf();				// Destructor
#ifdef TEST
void init(char **bufs, int n_pages, int tSize, int temp_fd, int buffer = FALSE);
#else
void init(char **bufs, int n_pages, int tSize,
		 HeapFile* temp_fd, int buffer = FALSE);
#endif

//   void init(char **bufs, int n_pages, int tSize, int temp_fd, int buffer = FALSE);

   char* Put(char *buf);		// Writes a tuple to the output buffer
   int flush(void);			// returns the # of tuples written.

 private:
   int dirty;				// Does this buffer contain dirty pages?
   int  t_per_pg,			// # of tuples that fit in 1 page
        t_in_buf;			// # of tuples that fit in the buffer
   int  t_wr_to_pg,			// # of tuples written to current page
        t_wr_to_buf;			// # of tuples written to buffer.
   int  curr_page;			// Current page being written to.
   char **_bufs;			// Array of pointers to buffer pages.
   int  _n_pages;			// number of pages in array
   int  t_size;				// Size of a tuple
   long t_written;			// # of tuples written so far.
   #ifdef TEST
   int  _temp_fd;			// fd of a temporary file
   #else               // ifdef added by BK
   HeapFile* _temp_fd;
   #endif
   int buffer_only;
};


class I_buf {
 public:
   I_buf();
   ~I_buf();
   void init(Iterator * pIter, char **bufs, int n_pages, int tSize);
   Status Get(char * & buf);
   int empty(void);
 private:
   int readin(void);
   
   char **_bufs;
   Iterator *_pIter;
   int    _n_pages;
   int    t_size;
   
   int    t_proc, t_in_buf;
   int    t_rd_from_pg, curr_page;
   int    t_per_pg;
   int   done;
   Tuple *op_tuple;
};


class IO_buf {
 public:
   IO_buf();				// Constructor - use init to initialize
   ~IO_buf();				// Destructor

#ifdef TEST
void init(char **bufs, int n_pages, int tSize, int temp_fd);
#else
void init(char **bufs, int n_pages, int tSize, HeapFile* temp_fd);
#endif


   void Put(Tuple *buf);		// Writes a tuple to the output buffer
   Status Get(Tuple * & buf);
   int flush(void);			// returns the # of tuples written.
   void reread(void);

 private:
   int done;
   int dirty;				// Does this buffer contain dirty pages?
   int  t_per_pg,			// # of tuples that fit in 1 page
        t_in_buf;			// # of tuples that fit in the buffer
   int  t_wr_to_pg,			// # of tuples written to current page
        t_wr_to_buf;			// # of tuples written to buffer.
   int  curr_page;			// Current page being written to.
   char **_bufs;			// Array of pointers to buffer pages.
   int  _n_pages;			// number of pages in array
   int  t_size;				// Size of a tuple
   long t_written;			// # of tuples written so far.
   #ifdef TEST
   int  _temp_fd;			// fd of a temporary file
   #else               // ifdef added by BK
   HeapFile* _temp_fd;
   #endif
   int  flushed;			// TRUE => buffer has been flushed.
   int  mode;
   int  t_rd_from_pg;			// # of tuples read from current page
   spoofI_buf *i_buf;			// gets input from a temporary file
};


class rr_buf {
 public:
   rr_buf();
   ~rr_buf();
   void init(Iterator * pIter, char **bufs, int n_pages, int tSize);

   Status get_next(Tuple * & tuple);
   int empty(void);

   Status refill(void);
   void re_read(void);
   
 private:
   char **_bufs;
   Iterator *_pIter;
   int    _n_pages;
   int    t_size;
   
   int    t_proc, t_in_buf;
   int    t_rd_from_pg, curr_page;
   int    t_per_pg;
   int   done;
   char  *op_buf;
};

#endif






