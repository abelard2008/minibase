// =============================================================================
//
// sort-merge.C
//		This file contains an implementation of the sort merge join
// algorithm as described in the Shapiro paper. It makes use of the external
// sorting utility to generate runs, and then uses the iterator interface to
// get successive tuples for the final merge.
//

// #include "new_error.h"


#include <stdio.h>
#include <fcntl.h>
//#include "test.h"
#include "tuple.h"
#include "iterator.h"
#include "joins.h"
#include "sort.h"
#include "tuple_utils.h"
#include "projection.h"
#include "pred_eval.h"

#ifndef TEST
#include "join_errors.h"
#include "new_error.h"
#include "minirel.h"
#endif

sort_merge::sort_merge
(
 AttrType in1[],	// Array containing field types of R.
 int      len_in1,	// # of columns in R.
 short    s1_sizes[],
 AttrType in2[],	// Array containing field types of S.
 int      len_in2,	// # of columns in S.
 short    s2_sizes[],

 int      join_col_in1,	// The col of R to be joined with
 int      sortFld1Len,
 int      join_col_in2,	// the col of S.
 int      sortFld2Len,
		       
 int      amt_of_mem,	// IN PAGES
 
 Iterator *    am1,	// access method for left i/p to join
 Iterator *    am2,	// access method for right i/p to join
 
 int     in1_sorted,	// is am1 sorted?
 int     in2_sorted,	// is am2 sorted?
 TupleOrder order,
 
 CondExpr **outFilter,	// Ptr to the output filter
 FldSpec  * proj_list,
 int        n_out_flds,
 Status   & s
)
{
   // #########################################################################
   // Amit> I don't know how I am going to make use of the buffer pages.
   //       My original idea was to pass 1/2 the buffer pages to one sort
   //       routine, the other 1/2 to the other sort routine, and dynamically
   //       perform the merge of the two sorted runs.
   // #########################################################################
   s = OK;
   copy(len_in1, in1, in1_len, _in1);
   copy(len_in2, in2, in2_len, _in2);

   // Compute Jtypes
   AttrType *Jtypes;
   short    *ts_size;

   perm_mat = proj_list;
   nOutFlds = n_out_flds;
   setup_op_tuple(Jtuple, Jtypes,
		  in1, len_in1, in2, len_in2,
		  s1_sizes, s2_sizes, ts_size,
		  proj_list, nOutFlds);

// ---------------
   // Copy t2_str_sizes also.
   int n_strs2 = 0;

   for (int i = 0; i < len_in2; i++) if (_in2[i] == attrString) n_strs2++;
   inner_str_sizes = new short [n_strs2];
   assert(inner_str_sizes);
   for (int i = 0; i < n_strs2; i++)    inner_str_sizes[i] = s2_sizes[i];

// ---------------

   p_i1 = am1;
   p_i2 = am2;

   if (!in1_sorted)
      p_i1 = new Sort(in1, len_in1, s1_sizes, am1, join_col_in1,
		      order, sortFld1Len, amt_of_mem / 2, s);
   if (! in2_sorted)
      p_i2 = new Sort(in2, len_in2, s2_sizes, am2, join_col_in2,
		      order, sortFld2Len, amt_of_mem / 2, s);

   if (s != OK) return;

   OutputFilter = outFilter;
   _order       = order;
   jc_in1       = join_col_in1;
   jc_in2       = join_col_in2;
   get_from_in1 = TRUE;
   get_from_in2 = TRUE;

   // open io_bufs
   io_buf1 = new IO_buf;
   io_buf2 = new IO_buf;

   // Allocate memory for the temporary tuples
   TempTuple1 = (Tuple *) new char [Tuple::max_size];
   TempTuple2 = (Tuple *) new char [Tuple::max_size];
   tuple1 = (Tuple *) new char [Tuple::max_size];
   tuple2 = (Tuple *) new char [Tuple::max_size];

  #ifdef TEST
   assert(io_buf1   != NULL && io_buf2   != NULL &&
	  TempTuple1 != NULL && TempTuple2 != NULL &&
	  tuple1     != NULL && tuple2     != NULL);
  #else
   if(io_buf1  == NULL || io_buf2  == NULL ||
      TempTuple1 == NULL || TempTuple2==NULL ||
      tuple1 ==  NULL || tuple2 ==NULL) {
        s = MINIBASE_FIRST_ERROR( JOINS, JoinError::NEWFAILED );
        return;
    }
  #endif


  #ifndef TEST
    if( amt_of_mem < 2 ) {
        s = MINIBASE_FIRST_ERROR( JOINS, JoinError::LOW_MEMORY );
        return;
    }
  #endif

   TempTuple1->setHdr(in1_len, _in1, s1_sizes);
   tuple1->setHdr(in1_len, _in1, s1_sizes);
   TempTuple2->setHdr(in2_len, _in2, s2_sizes);
   tuple2->setHdr(in2_len, _in2, s2_sizes);

   t1_size = tuple1->size();
   t2_size = tuple2->size();

   process_next_block = TRUE;
   done               = FALSE;

   // Two buffer pages to store equivalence classes
   // NOTE -- THESE PAGES ARE NOT OBTAINED FROM THE BUFFER POOL
   // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   _n_pages = 2;
   _bufs = new char *[_n_pages];
   for (int i = 0; i < _n_pages; i++)
      _bufs[i] = new char [MINIBASE_PAGESIZE];

#ifdef TEST
   temp_file_fd1 = open(temp_file_name(), O_RDWR);
   temp_file_fd2 = open(temp_file_name(), O_RDWR);
   assert(temp_file_fd1 > 0);
   assert(temp_file_fd2 > 0);
#else
   // Open temporary heap files
   Status hf_s;
   temp_file_fd1 = new HeapFile(NULL, hf_s);
   temp_file_fd2 = new HeapFile(NULL, hf_s);
   if(hf_s != OK)
     {
       s = MINIBASE_CHAIN_ERROR( JOINS, hf_s );
       return;
     }

#endif
   sortFldType = _in1[jc_in1-1];

   // Now, that stuff is setup, all we have to do is a get_next !!!!
}


sort_merge::~sort_merge()
{
#ifdef TEST
   // Free the allocated memory
   int count;

   for (count = 0; count < _n_pages; count++)
      delete [] _bufs[count];
   delete [] _bufs;
#else
   // Whatever has to be done to call the buffer manager and return the pages.
   // No return is needed since we do not get any pages from the buffer manager.
   delete temp_file_fd1;
   delete temp_file_fd2;
#endif

   delete [] (char *) TempTuple1;
   delete [] (char *) TempTuple2;
   delete [] (char *) tuple1;
   delete [] (char *) tuple2;
   delete [] (char *) Jtuple;

   // delete children
   delete p_i1;
   delete p_i2;

   delete io_buf1;
   delete io_buf2;

#ifndef TEST

#endif   
}



Status sort_merge::get_next(Tuple * &tuple)
{
   // All this function has to do is to get 1 tuple from one of the Iterators
   // (from both initially), use the sorting order to determine which one
   // gets sent up.
   // Amit> Hmmm it seems that some thing more has to be done in order to account
   //       for duplicates.... => I am following Raghu's 564 notes in order to
   //       obtain an algorithm for this merging. Some funda about "equivalence classes"
   int    comp_res;
   Status s;

   if (done) return DONE;

   while (1)
   {
      if (process_next_block)
      {
	 process_next_block = FALSE;
	 if (get_from_in1)
	    if ((s = p_i1->get_next(tuple1)) == DONE)
	    {
	       done = TRUE;
	       return DONE;
	    }
	    else if (s != OK) return s;

	 if (get_from_in2)
	    if ((s = p_i2->get_next(tuple2)) == DONE)
	    {
	       done = TRUE;
	       return DONE;
	    }
	    else if (s != OK) return s;

	 get_from_in1 = get_from_in2 = FALSE;
	 // Note that depending on whether the sort order is ascending or descending,
	 // this loop will be modified.
	 comp_res = CompareTupleWithTuple(sortFldType, tuple1, jc_in1, tuple2, jc_in2);
	 while ((comp_res < 0 && _order == Ascending) ||
		(comp_res > 0 && _order == Descending))
	 {
	    if ((s = p_i1->get_next(tuple1)) == DONE)
	    {
	       done = TRUE;
	       return DONE;
	    }
	    else if (s != OK) return s;

	    comp_res = CompareTupleWithTuple(sortFldType, tuple1,
					     jc_in1, tuple2, jc_in2);
	 }

	 comp_res = CompareTupleWithTuple(sortFldType, tuple1,
					  jc_in1, tuple2, jc_in2);
	 while ((comp_res > 0 && _order == Ascending) ||
		(comp_res < 0 && _order == Descending))
	 {
	    if ((s = p_i2->get_next(tuple2)) == DONE)
	    {
	       done = TRUE;
	       return DONE;
	    }
	    else if (s != OK) return s;

	    comp_res = CompareTupleWithTuple(sortFldType, tuple1,
					     jc_in1, tuple2, jc_in2);
	 }

	 if (comp_res != 0)
	 {
	    process_next_block = TRUE;
	    continue;
	 }

	 memcpy((char *) TempTuple1, (char *) tuple1, t1_size);
 	 memcpy((char *) TempTuple2, (char *) tuple2, t2_size);

	 io_buf1->init(_bufs,       1, t1_size, temp_file_fd1);
	 io_buf2->init(&(_bufs[1]), 1, t2_size, temp_file_fd2);

	 while (CompareTupleWithTuple(sortFldType, tuple1,
				      jc_in1, TempTuple1, jc_in1) == 0)
	 {
	    // Insert tuple1 into io_buf1
	    io_buf1->Put(tuple1);
	    if (p_i1->get_next(tuple1) == DONE)
	    {
//	       process_next_block = TRUE;
	       get_from_in1       = TRUE;
	       break;
	    }
	 }

	 while (CompareTupleWithTuple(sortFldType, tuple2,
				      jc_in2, TempTuple2, jc_in2) == 0)
	 {
	    // Insert tuple2 into io_buf2
	    io_buf2->Put(tuple2);
	    if (p_i2->get_next(tuple2) == DONE)
	    {
//	       process_next_block = TRUE;
	       get_from_in2       = TRUE;
	       break;
	    }
	 }

	 // tuple1 and tuple2 contain the next tuples to be processed after this set.
	 // Now perform a join of the tuples in io_buf1 and io_buf2.
	 // This is going to be a simple nested loops join with no frills. I guess,
	 // it can be made more efficient, this can be done by a future 564 student.
	 
	 // Another optimization that can be made is to choose the inner and outer
	 // by checking the number of tuples in each equivalence class.

	 if (io_buf1->Get(TempTuple1) == DONE)		// Should not occur
	   std::cerr << "Equiv. class 1 in sort-merge has no tuples" << std::endl;

      }

      if (io_buf2->Get(TempTuple2) == DONE)
      {
	 if (io_buf1->Get(TempTuple1) == DONE)
	 {
	    process_next_block = TRUE;
	    continue;				// Process next equivalence class
	 }
	 else
	 {
	    io_buf2->reread();
	    io_buf2->Get(TempTuple2);
	 }
      }
      if (Eval(OutputFilter, TempTuple1, TempTuple2, _in1, _in2) == TRUE)
      {
	 Join(TempTuple1, _in1, in1_len,
	      TempTuple2, _in2, in2_len,
	      Jtuple, perm_mat, nOutFlds);
	 tuple = Jtuple;
	 return OK;
      }
   }
}

