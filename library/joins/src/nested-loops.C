// =============================================================================
//
// nested-loops.C
//		This file contains an implementation of the nested loops join
// algorithm as described in the Shapiro paper.
// The algorithm is extremely simple:
//
//	foreach tuple r in R do
//	    foreach tuple s in S do
//		if (ri == sj) then add (r, s) to the result.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "test.h"
//#include "example.h"
#include "tuple.h"
#include "iterator.h"
#include "tuple_utils.h"
#include "projection.h"
#include "pred_eval.h"
#include "joins.h"
#include "file_scan.h"



// ================================================================================
#ifdef TEST
// ---------------------------------------------------------------------------

//Type types[] = {attrInteger, attrInteger, attrInteger, attrReal, attrString};
//Type Jtypes[] = {attrInteger, attrInteger, attrInteger, attrReal, attrString, attrInteger, attrInteger, attrInteger, attrReal, attrString};

#endif


// ================================================================================

/*
#ifndef TEST
# include "Iterator.h"
# include "tuple.h"
#endif
*/


nested_loops_join::nested_loops_join
		    (AttrType    in1[],		// Array containing field types of R.
                     int     len_in1,		// # of columns in R.
		     short   t1_str_sizes[],
		     AttrType    in2[],		// Array containing field types of S.
		     int     len_in2,		// # of columns in S.
		     short   t2_str_sizes[],

		     int     amt_of_mem,	// IN PAGES

                     Iterator *    am1,		// access method for left input to join
		     std::string  relationName,	// access methd for right input to join

		     CondExpr **outFilter,	// Ptr to the output filter
		     CondExpr **rightFilter,	// Ptr to filter applied to right i/p
		     FldSpec  * proj_list,
		     int        n_out_flds,
		     Status   & s
 		    )
{
   // Amit> I do not want to use strdup since it may use malloc. And if malloc and
   //       new are used in the same program, it may screw things up.
   s = OK;
   copy(len_in1, in1, in1_len, _in1);
   copy(len_in2, in2, in2_len, _in2);

   outer = am1;
   relnName = new char [strlen(relationName.c_str())+1];
   strncpy(relnName, relationName.c_str(), strlen(relationName.c_str())+1);

   inner_tuple = (Tuple*) new char [Tuple::max_size];

   OutputFilter = outFilter;
   RightFilter  = rightFilter;

   n_buf_pgs    = amt_of_mem;
   inner = NULL;
   done  = FALSE;
   get_from_outer = TRUE;

   AttrType *Jtypes;
   short    *t_size;

   perm_mat = proj_list;
   nOutFlds = n_out_flds;
   setup_op_tuple(Jtuple, Jtypes,
		  in1, len_in1, in2, len_in2,
		  t1_str_sizes, t2_str_sizes, t_size,
		  proj_list, nOutFlds);

   _t2_str_sizes = t2_str_sizes;   // added by BK

#ifndef TEST
   hf = new HeapFile(relnName, s);
   if (s != OK)
       s = MINIBASE_CHAIN_ERROR( JOINS, s );
#endif


   delete [] Jtypes;
   delete [] t_size;
}


nested_loops_join::~nested_loops_join()
{
   delete [] (char *) Jtuple;
   delete [] relnName;
   delete [] _in1;
   delete [] _in2;
#ifndef TEST
   delete hf;
#endif

   if (inner != NULL)
    delete inner;
   delete outer;
   delete [] inner_tuple;
}


//
// Performs the get next tuple function.
//

Status nested_loops_join::get_next(Tuple * & tuple)
{
   // This is a DUMBEST form of a join, not making use of any key information...

   Status s;
   if (done)
      return DONE;

   do
   {
      // ================================================================================
      //
      // If get_from_outer is TRUE, Get a tuple from the outer, delete
      // an existing scan on the file, and reopen a new scan on the file.
      // If a get_next on the outer returns DONE?, then the nested loops join
      // is done too.

      if (get_from_outer == TRUE)
      {
	 get_from_outer = FALSE;
	 if (inner != NULL)     // If this not the first time,
	 {
     	    delete inner;       // close scan
	    inner = NULL;
         }

	 // Open a scan on the relnName 
#ifdef TEST
	 inner = new RelnScan(relnName, _in2, in2_len);
#else
	 inner = hf->openScan(s);
	 if (s != OK)
             return MINIBASE_CHAIN_ERROR( JOINS, s );
#endif
	 if ((s = outer->get_next(outer_tuple)) == DONE)
	 {
	    done = TRUE;

	    // added by BK
	    if (inner != NULL) 
	    {
		delete inner;      
		inner = NULL;
            }

	    return DONE;
	 }
	 else if (s != OK) return s;

      }  // ENDS: if (get_from_outer == TRUE)

      // ================================================================================
      //
      // The next step is to get a tuple from the inner,
      // while the inner is not completely scanned && there is no match (with pred),
      //      get a tuple from the inner.

#ifdef TEST
      while ((s = inner->get_next(inner_tuple)) != DONE)
#else
      RID rid;
      int length;
      while ((s = inner->getNext(rid, (char*)inner_tuple, length)) != DONE)
#endif
      {
	 if (s != OK) return s;

//	 std::cerr << "\tInner: ";  inner_tuple->print(_in2);
//	 std::cerr << "\tOuter: ";  outer_tuple->print(_in1);
	 
	 if (Eval(RightFilter, inner_tuple, NULL, _in2, NULL) == TRUE)
	 {
	    if (Eval(OutputFilter, outer_tuple, inner_tuple, _in1, _in2) == TRUE)
	    {
	       // Apply a projection on the outer and inner tuples.
	       Join(outer_tuple, _in1, in1_len,
		    inner_tuple, _in2, in2_len,
		    Jtuple, perm_mat, nOutFlds);

	       // return the resulting tuple.
	       tuple = Jtuple;
	       return OK;
	    }
	 }
      }

      // ================================================================================
      //
      // There has been no match. (otherwise, we would have returned from the while loop.
      // Hence, inner is exhausted, => set get_from_outer = TRUE, go to top of loop

      get_from_outer = TRUE;	// Loop back to top and get next outer tuple.

   }
   while (1);
   
}

