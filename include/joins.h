#ifndef __JOINS__  
#define __JOINS__

//
// Prototypes for the hash joins are available in hash_joins.h
//

#define int int  // Moronic idea.. (Ranjani thinks so).

#include "tuple.h"
#include "iterator.h"
#include "query.h"
#include "sort.h"
#include "io_bufs.h"
#include <string>
#include "index.h"
//
// This file contains the interface for the joins.
// We name the two relations being joined as R and S.
//

/*
class nested_loops_join : public Iterator
{
private:

 public:
   nested_loops_join(AttrType    in1[],		// Array containing field types of R.
                     int     len_in1,		// # of columns in R.
		     short   t1_str_sizes[],
		     AttrType    in2[],		// Array containing field types of S.
		     int     len_in2,		// # of columns in S.
		     short   t2_str_sizes[],

		     int     amt_of_mem,	// IN PAGES

                     Iterator *    am1,		// access method for left i/p to join
		     char *  relationName,	// access method for right i/p to join

		     CondExpr **outFilter,	// Ptr to the output filter
		     CondExpr **rightFilter,	// Ptr to filter applied on right i/p
		     FldSpec  * proj_list,
		     int        n_out_flds,
		     Status   & s
		    );
   Status get_next(Tuple * &tuple);		// The tuple is returned.

   ~nested_loops_join();
};

*/

class nested_loops_join : public Iterator
{
private:
   AttrType     * _in1, * _in2;
   int        in1_len, in2_len;
   Iterator * outer;
#ifdef TEST   
   Iterator * inner;			// iterators for inner and outer relns
#else
   HeapFile * hf;
   Scan     * inner;
#endif

   char *     relnName;				// Store the name of the relation to scan.
   short*     _t2_str_sizes;     // added by BK
   CondExpr * * OutputFilter;
   CondExpr * * RightFilter;
   int        n_buf_pgs;			// # of buffer pages available.
   int       done,				// Is the join complete
              get_from_outer;			// if TRUE, a tuple is got from outer
   Tuple *    outer_tuple, *inner_tuple;
   Tuple *    Jtuple;				// Joined tuple
   FldSpec *  perm_mat;
   int        nOutFlds;

 public:
   nested_loops_join(AttrType    in1[],		// Array containing field types of R.
                     int     len_in1,		// # of columns in R.
		     short   t1_str_sizes[],
		     AttrType    in2[],		// Array containing field types of S.
		     int     len_in2,		// # of columns in S.
		     short   t2_str_sizes[],

		     int     amt_of_mem,	// IN PAGES

                     Iterator *    am1,		// access method for left i/p to join
                     std::string relationName,	// access method for right i/p to join

		     CondExpr **outFilter,	// Ptr to the output filter
		     CondExpr **rightFilter,	// Ptr to filter applied on right i/p
		     FldSpec  * proj_list,
		     int        n_out_flds,
		     Status   & s
		    );
   Status get_next(Tuple * &tuple);		// The tuple is returned.

   ~nested_loops_join();
};






class index_nested_loop : public Iterator
{
private:
   AttrType     * _in1, * _in2;
   int        in1_len, in2_len;
   Iterator * outer;
#ifdef TEST
   Iterator * inner;                    // iterators for inner and outer relns
#else
   HeapFile * hf;
   IndexFile* Index;
   IndexFileScan* Iscan;
#endif

   char *     relnName;                         // Store the name of the relation to scan.
   char*      inName;            // added by BK
   short*     _t2_str_sizes;     // added by BK
   CondExpr * * OutputFilter;
   CondExpr * * RightFilter;
   int        n_buf_pgs;                        // # of buffer pages available.
   int       done,                             // Is the join complete
              get_from_outer;                   // if TRUE, a tuple is got from outer
   Tuple *    outer_tuple, *inner_tuple;
   Tuple *    Jtuple;                           // Joined tuple
   FldSpec *  perm_mat;
   int        nOutFlds;

// BK
   int jc1, jc2;
   IndexType inType;

   int index_only;
   


 public:
   index_nested_loop(AttrType    in1[],		// Array containing field types of R.
                     int     len_in1,		// # of columns in R.
		     short   t1_str_sizes[],
		     AttrType    in2[],		// Array containing field types of S.
		     int     len_in2,		// # of columns in S.
		     short   t2_str_sizes[],

		     int     join_col_in1,	// The col of R to be joined with
		     int     join_col_in2,	// the col of S.

		     int     amt_of_mem,	// IN PAGES

                     Iterator *    am1,		// access method for left input to join.
		     IndexType in_type,         // the index type
                     std::string  index_name,	// access method for right input to join.
                     std::string  relationName,	// access method for right input to join.

		     CondExpr * * outFilter,	// Ptr to the output filter
		     CondExpr * * rightFilter,	// Ptr to a filter applied on the right i/p
		     FldSpec  * proj_list,
		     int        nOutFlds,
		     int indexOnly,
		     Status   & s
		    );

   Status get_next(Tuple * &tuple);		// The tuple is returned.
   ~index_nested_loop();
};

// ====================================================================================
//
class sort_merge : public Iterator
{
 private:
   AttrType * _in1, *_in2;
   int        in1_len, in2_len;
   Iterator * p_i1,				// pointers to the two iterators. If the
            * p_i2;				// inputs are sorted, then no sorting is done
   TupleOrder  _order;				// The sorting order.
   CondExpr * * OutputFilter;

   int       get_from_in1, get_from_in2;	// state variables for get_next
   int        jc_in1, jc_in2;
   int        process_next_block;
   short    * inner_str_sizes;
   IO_buf   * io_buf1, * io_buf2;
   Tuple    * TempTuple1, * TempTuple2;
   Tuple    * tuple1, * tuple2;
   int       done;
   char    ** _bufs;
   int        _n_pages;
#ifdef TEST
   int        temp_file_fd1, temp_file_fd2;
#else
   HeapFile *temp_file_fd1, *temp_file_fd2;
#endif
   AttrType   sortFldType;
   int        t1_size, t2_size;
   Tuple    * Jtuple;
   FldSpec *  perm_mat;
   int        nOutFlds;
 public:
   sort_merge(AttrType    in1[],		// Array containing field types of R.
	      int     len_in1,			// # of columns in R.
	      short   t1_str_sizes[],
	      AttrType    in2[],		// Array containing field types of S.
	      int     len_in2,			// # of columns in S.
	      short   t2_str_sizes[],
	      
	      int     join_col_in1,		// The col of R to be joined with
	      int      sortFld1Len,
	      int     join_col_in2,		// the col of S.
	      int      sortFld2Len,
	      
	      int     amt_of_mem,		// IN PAGES

	      Iterator *    am1,		// access method for left input to join.
	      Iterator *    am2,		// access method for right input to join.

	      int     in1_sorted,		// is am1 sorted?
	      int     in2_sorted,		// is am2 sorted?
	      TupleOrder order,

	      CondExpr * * outFilter,		// Ptr to the output filter
	      FldSpec  * proj_list,
	      int        nOutFlds,
	      Status   & s
	     );

   Status get_next(Tuple * &tuple);		// The tuple is returned.

   ~sort_merge();
};


/*
class sort_merge : public Iterator
{
 private:
 public:
   sort_merge(AttrType    in1[],		// Array containing field types of R.
	      int     len_in1,			// # of columns in R.
	      short   t1_str_sizes[],
	      AttrType    in2[],		// Array containing field types of S.
	      int     len_in2,			// # of columns in S.
	      short   t2_str_sizes[],
	      
	      int     join_col_in1,		// The col of R to be joined with
	      int      sortFld1Len,
	      int     join_col_in2,		// the col of S.
	      int      sortFld2Len,
	      
	      int     amt_of_mem,		// IN PAGES

	      Iterator *    am1,		// access method for left input to join.
	      Iterator *    am2,		// access method for right input to join.

	      int     in1_sorted,		// is am1 sorted?
	      int     in2_sorted,		// is am2 sorted?
	      TupleOrder order,

	      CondExpr * * outFilter,		// Ptr to the output filter
	      FldSpec  * proj_list,
	      int        nOutFlds,
	      Status   & s
	     );

   Status get_next(Tuple * &tuple);		// The tuple is returned.

   ~sort_merge();
};

*/

#endif

