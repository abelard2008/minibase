#include <stdio.h>
#include <ostream>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

//#include "test.h"
#include "tuple.h"
#include "new_error.h"
#include "fileIspoof.h"

extern "C" {
     long tell(int fd);
}

// ========================================================================
//
// Empty constructor -- all the work is done by the init function
//

spoofI_buf::spoofI_buf()
{ dummy = NULL;
#ifndef TEST
 hf_scan = NULL;
#endif
 }

spoofI_buf::~spoofI_buf()
{ delete [] dummy; }


// ========================================================================
//
// init - This function performs the initialization of the input buffer.
#ifdef TEST
void spoofI_buf::init(int fd, char **bufs, int n_pages, int tSize, int Ntuples)
#else
void spoofI_buf::init(HeapFile* fd, char **bufs, int n_pages,
		      int tSize, int Ntuples, Status &s)
#endif
{
   _fd       = fd;       _bufs        = bufs;
   _n_pages  = n_pages;  t_size       = tSize;

   t_proc    = 0;        t_in_buf     = 0;
   tot_t_proc= 0;
   curr_page = 0;        t_rd_from_pg = 0;
   done      = FALSE;    t_per_pg     = MINIBASE_PAGESIZE / t_size;

   if (op_tuple != NULL) delete [] dummy;
   op_tuple  = (Tuple *) dummy;

#ifdef TEST
   n_tuples = Ntuples;
   lseek(_fd, 0L, SEEK_SET);		// Or equivalent
#else
//   n_tuples = fd->getRecCnt();
   n_tuples = Ntuples;
// BK
  
   // open a scan
   if (hf_scan != NULL) delete hf_scan;
   hf_scan = _fd->openScan(s);
//   error("spoofI_buf::init() - unimplemented.");
#endif
}


// ====================================================================
//

int spoofI_buf::empty()
{
   if (tot_t_proc == n_tuples) done = TRUE;
   return done;
}


// ====================================================================
//
// This the heart of the input buffer. The readin() function is used
// to fill the input buffer with data. The rest of the function is
// fairly self explanatory.
//
// ====================================================================

Status spoofI_buf::Get(Tuple * & buf)
{
   if (tot_t_proc == n_tuples) done = TRUE;

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

   memcpy(buf, _bufs[curr_page] + t_rd_from_pg * t_size, t_size);
   tot_t_proc++;

   // Setup for next read
   t_rd_from_pg++; t_proc++;
   if (t_rd_from_pg == t_per_pg)
   {
      t_rd_from_pg = 0; curr_page++;
   }

   return OK;
}


// =====================================================================
//
// readin - this function uses the iterator to get tuples an fill up
//          the input buffer. It returns the number of tuples read from
//          the iterator. This function uses the curr_page variable --
//          this should not cause any problems since it is reset after
//          a call to this function.
//
// =====================================================================

int spoofI_buf::readin(void)
{
   int   t_read = 0, tot_read = 0;
   char *t      = new char [t_size + 1];
#ifdef TEST
   int   n_read = 0,  padding = 0;
#endif

   curr_page = 0;
   while (curr_page < _n_pages)
   {
      while (t_read < t_per_pg)
      {
#ifdef TEST
	 n_read = read(_fd, t, t_size);
	 if (n_read != t_size)
	    return tot_read;	// return # of tuples read
#else
      // These 3 lines replace the commented out line.  BK
	 RID rid;
	 int recLen;
	 if (hf_scan->getNext(rid, t, recLen) == DONE) return tot_read;
	 // if (hf_scan->getNext(t) == DONE) return tot_read;
#endif
	 memcpy(_bufs[curr_page] + t_read * t_size, t, t_size);
	 t_read++; tot_read++;
      }
#ifdef TEST
      // Now, there is some extra space at the end of the page which is smaller
      // than a tuple, but nevertheless MUST be eaten up
      padding = MINIBASE_PAGESIZE - t_per_pg * t_size;
      n_read = read(_fd, t, padding);
      assert(n_read == padding);
#endif
      t_read     = 0;
      curr_page++;
   }
   delete [] t;
   return tot_read;
}


#ifdef TEST
char * temp_file_name(void)
{
   static char f_name[] = "run_XXXX";
   char system_str[20];
   static int counter = 0;

   sprintf(f_name+4, "%04d", counter++);
   sprintf(system_str, "touch %s", f_name);
   system(system_str);
   return f_name;
}

#endif
