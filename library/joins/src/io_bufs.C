#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

//#include "test.h"
#include "iterator.h"
#include "tuple.h"
#include "new_error.h"
#include "io_bufs.h"


extern "C" {
     long tell(int fd);
}

/*****************************************

O_buf = used to buffer lots of tuples
I_buf = unused


*****************************************/

// Default constructor.
O_buf::O_buf(){}

// destructor.
O_buf::~O_buf(){}

/***********************************************************
Comments added by BK

O_buf::Put takes tuples and stores them on the buffer pages that
were passed to O_buf::init.  O_buf::flush inserts them enmass into
a temporary HeapFile.

***********************************************************/

// =============================================================================
//
// O_buf is an output buffer. It takes as input:
//	PIter	-	a pointer to a stream of data.  // <= isn't used?? BK
//	bufs	-	a pointer to an array of pointers to buffer pages.
//			(EACH ELEMENT IS A SINGLE BUFFER PAGE).
//	n_pages	-	the number of pages
//	tSize	-	tuple size
//	temp_fd	-	fd of a temporary file (or HeapFile - BK)
//	buffer	-	TRUE => it is used as a buffer => if it is flushed, print
//			a nasty message. it is FALSE by default.
//
// =============================================================================

#ifdef TEST
void O_buf::init(char **bufs, int n_pages, int tSize, int temp_fd, int buffer)
#else
void O_buf::init(char **bufs, int n_pages, int tSize,
		 HeapFile* temp_fd, int buffer)
#endif
{
   _bufs    = bufs;
   _n_pages = n_pages;
   t_size   = tSize;
   _temp_fd = temp_fd;

   dirty       = FALSE;
   t_per_pg    = MINIBASE_PAGESIZE / t_size;
   t_in_buf    = n_pages * t_per_pg;
   t_wr_to_pg  = 0;
   t_wr_to_buf = 0;
   t_written   = 0L;
   curr_page   = 0;
   buffer_only = buffer;
}

// returns NULL on error
char * O_buf::Put(char * buf)
{
   char *tuple_ptr;

   memcpy(_bufs[curr_page] + t_wr_to_pg * t_size, buf, t_size);
   tuple_ptr = _bufs[curr_page] + t_wr_to_pg * t_size;

   t_written++; t_wr_to_pg++; t_wr_to_buf++; dirty = TRUE;

   if (t_wr_to_buf == t_in_buf)		// Buffer full?
   {
      flush();				// Flush it
      
//#ifndef TEST
 //     if (error) return NULL;
//#endif
      t_wr_to_pg = 0; t_wr_to_buf = 0;	// Initialize page info
      curr_page  = 0;
   }
   else if (t_wr_to_pg == t_per_pg)
   {
      t_wr_to_pg = 0;
      curr_page++;
   }

   return tuple_ptr;
}


int O_buf::flush(void)
{
   int count;
#ifdef TEST
   int bytes_written = 0;
#endif

   if (buffer_only == TRUE)
      //error("Buffer is being flushed!!!!! -- ERROR condition");
      printf("Stupid error - but no error protocol\n");

   if (dirty)
   {
      for (count = 0; count <= curr_page; count++)
      {
#ifdef TEST
         bytes_written = write(_temp_fd, _bufs[count], MINIBASE_PAGESIZE);
	 assert(bytes_written == MINIBASE_PAGESIZE);
#else
	 RID rid;
	 Status s;
	 // Will have to go thru entire buffer writing tuples to disk

	 if (count == curr_page)
	   for (int i = 0; i < t_wr_to_pg; i++)
	   {
	      s = _temp_fd->insertRecord(_bufs[count] + t_size * i, t_size, rid);
	      if (s != OK) { /* error = s;*/ return -1; }
	   }
         else
	   for (int i = 0; i < t_per_pg; i++)
	   {
	     s = _temp_fd->insertRecord(_bufs[count] + t_size * i, t_size, rid);
	     if (s != OK) { /* error = s;*/ return -1; }
           }
#endif
      }

      dirty = FALSE;
   }

   return t_written;
}

#if 0
               UNUSED
// ===========================================================================
//
// Empty constructor -- all the work is done by the init function
//

I_buf::I_buf()
{}

I_buf::~I_buf()
{}


// ==========================================================================
//
// init - This function performs the initialization of the input buffer.

void I_buf::init(Iterator * pIter, char **bufs, int n_pages, int tSize)
{
   _pIter    = pIter;    _bufs        = bufs;
   _n_pages  = n_pages;  t_size       = tSize;

   t_proc    = 0;        t_in_buf     = 0;
   curr_page = 0;        t_rd_from_pg = 0;
   done      = FALSE;    t_per_pg     = MINIBASE_PAGESIZE / t_size;
   op_tuple  = (Tuple *) new char [Tuple::max_size];
}


// ========================================================================
//

int I_buf::empty()
{ return done; }


// ========================================================================
//
// This the heart of the input buffer. The readin() function is used to fill the input
// buffer with data. The rest of the function is fairly self explanatory.
//
// ========================================================================

Status I_buf::Get(char * & buf)
{
   if (done == TRUE) return DONE;
   if (t_proc == t_in_buf)
   {
      t_in_buf = readin();
      curr_page = 0; t_rd_from_pg = 0; t_proc = 0;
   }
   if (t_in_buf == 0)			// No tuples read in?
   {
      done = TRUE; return DONE;
   }

   memcpy((char *) op_tuple, _bufs[curr_page] + t_rd_from_pg * t_size, t_size);
   buf = (char *) op_tuple;

   // Setup for next read
   t_rd_from_pg++; t_proc++;
   if (t_rd_from_pg == t_per_pg)
   {
      t_rd_from_pg = 0; curr_page++;
   }

   return OK;
}


// ==================================================================================
//
// readin - this function uses the iterator to get tuples an fill up the input buffer.
//          It returns the number of tuples read from the iterator.
//          This function uses the curr_page variable -- this should not cause any
//          problems since it is reset after a call to this function.
//
// ==================================================================================

int I_buf::readin(void)
{
   int    t_read = 0, tot_read = 0;
   Tuple *t      = NULL;
   int    t_wr_to_pg = 0;

   curr_page = 0;
   while (curr_page < _n_pages)
   {
      while (t_read < t_per_pg)
      {
	 if (_pIter->get_next(t) == DONE) return tot_read;
	 memcpy(_bufs[curr_page] + t_read * t_size, (char *) t, t->size());
	 t_read++; tot_read++;
      }
      t_read     = 0;
      t_wr_to_pg = 0;
      curr_page++;
   }
   return tot_read;
}

#endif		// #if 0

// ============================================================

#define WRITE_BUFFER		0
#define READ_BUFFER		1

// ============================================================
// Default constructor.

IO_buf::IO_buf(){}

// destructor.
IO_buf::~IO_buf(){}

// =============================================================================
//
// IO_buf is an output, input buffer. First tuples are written to it, and then they
// are read out from it. init() takes as input:
//


//                                            read?
//                                             \/
// It is implicit in the logic that the first write to this buffer causes it to
// become a read only buffer, it will not allow you to write to it. For reuse,
// destroy the object and create another instance.
//
//	bufs	-	a pointer to an array of pointers to buffer pages.
//			(EACH ELEMENT IS A SINGLE BUFFER PAGE).
//	n_pages	-	the number of pages
//	tSize	-	tuple size
//	temp_fd	-	fd of a temporary file
//
// =============================================================================

#ifdef TEST
void IO_buf::init(char **bufs, int n_pages, int tSize, int temp_fd)
#else
void IO_buf::init(char **bufs, int n_pages, int tSize, HeapFile* temp_fd)
#endif
{
   _bufs    = bufs;
   _n_pages = n_pages;
   t_size   = tSize;
   _temp_fd = temp_fd;

   dirty       = FALSE;
   t_per_pg    = MINIBASE_PAGESIZE / t_size;
   t_in_buf    = n_pages * t_per_pg;
   t_wr_to_pg  = 0;
   t_wr_to_buf = 0;
   t_written   = 0L;
   curr_page   = 0;
   flushed     = FALSE;
   mode        = WRITE_BUFFER;

/*
#ifndef TEST 
std::cout << "Serious problem in IO::init in io_bufs.C\n\n";   // I fixed it by just using an ifdef
#endif                                                    // for the lseek pointer adjustment
*/

#ifdef TEST   
lseek(_temp_fd, 0L, SEEK_SET);
#endif

   i_buf       = new spoofI_buf;
   assert(i_buf);
   done        = FALSE;
}


void IO_buf::Put(Tuple * buf)
{
   if (mode != WRITE_BUFFER)
      //error("Trying to write to io buffer when it is acting as a input buffer");
	printf("Error\n");

   memcpy(_bufs[curr_page] + t_wr_to_pg * t_size, buf, t_size);

   t_written++; t_wr_to_pg++; t_wr_to_buf++; dirty = TRUE;

   if (t_wr_to_buf == t_in_buf)		// Buffer full?
   {
      flush();				// Flush it
#ifndef TEST
// WHAT IS THIS???      if (error) return;
#endif
      t_wr_to_pg = 0; t_wr_to_buf = 0;	// Initialize page info
      curr_page  = 0;
   }
   else if (t_wr_to_pg == t_per_pg)
   {
      t_wr_to_pg = 0;
      curr_page++;
   }

   return;
}


int IO_buf::flush(void)
{
   int count;
#ifdef TEST
   int bytes_written = 0;
#endif

   flushed = TRUE;
   if (dirty)
   {
      for (count = 0; count <= curr_page; count++)
      {
#ifdef TEST
         bytes_written = write(_temp_fd, _bufs[count], MINIBASE_PAGESIZE);
	 assert(bytes_written == MINIBASE_PAGESIZE);
#else
	 RID rid;
	 Status s;
	 // Will have to go thru entire buffer writing tuples to disk
	 for (int i = 0; i < t_wr_to_pg; i++)
	 {
	    s = _temp_fd->insertRecord(_bufs[count] + t_size * i, t_size, rid);
	    if (s != OK) { /* error = s;*/ return -1; }
	 }
#endif
      }
      dirty = FALSE;
   }

   return t_written;
}


void IO_buf::reread(void)
{
  Status status;

   mode = READ_BUFFER;
   if (flushed)			// Has the output buffe rbeen flushed?
   {
      // flush all the remaining tuples to disk.
      flush();
      
      // Open a scan on the file.
      #ifdef TEST
      i_buf->init(_temp_fd, _bufs, _n_pages, t_size, t_written);
      #else
      i_buf->init(_temp_fd, _bufs, _n_pages, t_size, t_written, status);
      #endif
   }
   else
   {
      // All the tuples are in the buffer, just read them out.
      t_rd_from_pg = 0;
      curr_page    = 0;
   }
}   


Status IO_buf::Get(Tuple * & buf)
{
   Status s;

   if (done)
      return DONE;

   if (mode == WRITE_BUFFER)		// Switching from writing to reading?
      reread();

   if (flushed)
   {
      // get tuples from 
      if ((s = i_buf->Get(buf)) == DONE)
      {
	 done = TRUE;
	 return DONE;
      }
      else if (s != OK) return s;
   }
   else
   {
      // just reading tuples from the buffer pages.
      if ((curr_page * t_per_pg + t_rd_from_pg) == t_written)
      {
	 done = TRUE;
	 return DONE;
      }
      memcpy(buf, _bufs[curr_page] + t_rd_from_pg * t_size, t_size);

      // Setup for next read
      t_rd_from_pg++;
      if (t_rd_from_pg == t_per_pg)
      {
	 t_rd_from_pg = 0; curr_page++;
      }
   }

   return OK;
}


// ===================================================================================
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// =============================== refill_reread_buf =================================
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ===================================================================================


rr_buf::rr_buf()
{ op_buf = NULL; }

rr_buf::~rr_buf()
{ delete [] op_buf; }


// =========================================================================
//
// init - This function performs the initialization of the input buffer.

void rr_buf::init(Iterator * pIter, char **bufs, int n_pages, int tSize)
{
   _pIter    = pIter;    _bufs        = bufs;
   _n_pages  = n_pages;  t_size       = tSize;

   op_buf = new char [tSize];
   t_proc    = 0;        t_in_buf     = 0;
   curr_page = 0;        t_rd_from_pg = 0;
   done      = FALSE;    t_per_pg     = MINIBASE_PAGESIZE / t_size;
}


// =========================================================================
//

int rr_buf::empty()
{ return done; }


// ==========================================================================
//
// This the heart of the refill reread buffer. The readin() function is used to fill the
// input buffer with data. The rest of the function is fairly self explanatory.
//
// ===========================================================================

Status rr_buf::get_next(Tuple * & tuple)
{
   if (done == TRUE) return DONE;
   if (t_proc == t_in_buf)
   {
      done = TRUE;
      return DONE;
   }
   if (t_in_buf == 0) { done = TRUE; return DONE;}	// No tuples read in?

   memcpy(op_buf, _bufs[curr_page] + t_rd_from_pg * t_size, t_size);
   tuple = (Tuple *) op_buf;

   // Setup for next read
   t_rd_from_pg++; t_proc++;
   if (t_rd_from_pg == t_per_pg)
   {
      t_rd_from_pg = 0; curr_page++;
   }

   return OK;
}


// =================================================================================
//
// refill - this function uses the iterator to get tuples an fill up the rr buffer.
//          It returns the number of tuples read from the iterator.
//          This function uses the curr_page variable -- this should not cause any
//          problems since it is reset after a call to this function.
//
// ==================================================================================

Status rr_buf::refill(void)
{
   int    t_read = 0, tot_read = 0;
   Tuple *t      = NULL;
   int    t_wr_to_pg = 0;

   curr_page = 0;
   while (curr_page < _n_pages)
   {
      while (t_read < t_per_pg)
      {
	 if (_pIter->get_next(t) == DONE)
	 {
	    if (t_read == 0)
	    {
	       done = TRUE;
	       return DONE;
	    }
	    else
	    {
	       curr_page = 0; t_rd_from_pg = 0; t_proc = 0;
	       t_in_buf = tot_read;
	       done = FALSE;
	       return OK;
	    }
	 }

	 memcpy(_bufs[curr_page] + t_read * t_size, (char *) t, t->size());
	 t_read++; tot_read++;
      }
      t_read     = 0;
      t_wr_to_pg = 0;
      curr_page++;
   }

   curr_page = 0; t_rd_from_pg = 0; t_proc = 0;
   t_in_buf = tot_read;
   done = FALSE;

   return OK;
}



void rr_buf::re_read(void)
{
   curr_page = 0; t_rd_from_pg = 0; t_proc = 0;
   done = FALSE;
}

