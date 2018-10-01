#ifndef __DUPL_ELIM__
#define __DUPL_ELIM__

#include "iterator.h"
#include "io_bufs.h"
#include "fileIspoof.h"
// #include "sort.h"

class DuplElim : public Iterator
{
 private:
   AttrType *_in;			// memory for array allocated by constructor
   int       in_len;
   short    *str_lens;

   Iterator *_am;
   int      done;

   AttrType  sortFldType;
   int       sortFldLen;
   Tuple *Jtuple;

   Tuple *TempTuple1, *TempTuple2;

 public:
   DuplElim(AttrType in[],		// Array containing field types of R.
	    int      len_in,		// # of columns in R.
	    short    str_sizes[],
	    Iterator *am,		// access method for left input to join
	    int       amt_of_mem,	// IN PAGES
	    int     inp_sorted,
	    Status  & s
           );

   Status get_next(Tuple * &tuple);	// The tuple is returned.

   ~DuplElim();
};



#endif

