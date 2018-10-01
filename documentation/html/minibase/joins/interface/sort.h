SORT HEADER FILE
----------------

#ifndef __SORT__
#define __SORT__

#include "pq_defs.h"
#include "pnode.defs.h"
#include "pnode.SplayNode.h"
#include "pnode.SplayPQ.h"

#include "iterator.h"
#include "io_bufs.h"
#include "fileIspoof.h"

#define FLOAT_MIN       1.0e-25
#define FLOAT_MAX       1.0e+25

class Sort : public Iterator
{
 private:
   AttrType *_in;			// memory for array allocated by constructor
   int       n_cols;
   short    *str_lens;
   Iterator *_am;
   int       _sort_fld;
   TupleOrder order;
   int       _n_pages;
   char **   bufs;
   int      first_time;
   int       Nruns;
   int       max_elems_in_heap;
   int       sortFldLen;
   int       tuple_size;

   pnodeSplayPQ *Q;
#ifdef TEST
   int * temp_files;
#else
   HeapFile ** temp_files; 
#endif
   int   n_tempfiles;
   char *output_tuple;
   int * n_tuples;
   int   n_runs;
   Tuple *op_buf;
   O_buf o_buf;
   spoofI_buf *i_buf;

   int* bufs_pids;

   void setup_for_merge(int tuple_size, int n_R_runs);
#ifdef TEST
   int generate_runs(int max_elems, AttrType sortFldType, int sortFldLen);
#else
   int generate_runs(int max_elems, AttrType sortFldType, int sortFldLen, Status &s);
#endif
   
   char * delete_min(void);

 public:
   Sort(AttrType in[],			// Array containing field types of R.
	int      len_in,		// # of columns in R.
	short    str_sizes[],

	Iterator *am,			// access method for left input to join.

	int       fld_no,		// the field number of the field to sort on.
	TupleOrder sort_order,		// ASCENDING, DESCENDING
	int       sort_field_len,	// length of the sort field.
	int       amt_of_mem,		// IN PAGES
	Status  & s
       );

   Status get_next(Tuple * &tuple);	// The tuple is returned.

   ~Sort();
};



#endif

