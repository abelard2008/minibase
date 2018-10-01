// =============================================================================
//
// index-nl.C
//		This file contains an implementation of the index nested loops
// algorithm as described in Raghu' lecture notes.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "test.h"
#include "tuple.h"
#include "iterator.h"
#include "tuple_utils.h"
#include "projection.h"
#include "pred_eval.h"

#include "joins.h"
#include "join_errors.h"
#include "index.h"
/*  #include "linearhashing.h"  */
#include "btfile.h"

#include "index_scan.h"   // allows use of IndexUtils which create BTreeFileScan object

static const char* joinErrMsgs[] = {
    "not enough memory to do a hash join",
    "join attribute types do not match",
    "estimate for pages of relation-1 is incorrect",
    "error in hash function in hash join",
    "new failed in joins",
    "internal error(bug?) in the joins",
    "bad scan",
    "can't do an IndexScan on a NONE index",
    "unknown index type in IndexScanIter",
};

static error_string_table err_tab( JOINS, joinErrMsgs );

// ================================================================================



index_nested_loop::index_nested_loop
		    (AttrType    in1[],		// Array containing field types of R.
                     int     len_in1,		// # of columns in R.
		     short   t1_str_sizes[],
		     AttrType    in2[],		// Array containing field types of S.
		     int     len_in2,		// # of columns in S.
		     short   t2_str_sizes[],

		     int     join_col_in1,	// The col of R to be joined with
		     int     join_col_in2,	// the col of S.

		     int     amt_of_mem,	// IN PAGES

                     Iterator *    am1,		// access method for left input to join
		     IndexType in_type,		// the index type
		     std::string  index_name,	// access method for right input to join.
		     std::string  relationName,	// access methd for right input to join

		     CondExpr **outFilter,	// Ptr to the output filter
		     CondExpr **rightFilter,	// Ptr to filter applied to right i/p
		     FldSpec  * proj_list,
		     int        n_out_flds,
		     int indexOnly,
		     Status   & s
 		    )
{
   // Amit> I do not want to use strdup since it may use malloc. And if malloc and
   //       new are used in the same program, it may screw things up.

   s = OK;
   jc1    = join_col_in1;
   jc2    = join_col_in2;
   inType = in_type;
   Index  = NULL;
   Iscan  = NULL;

   copy(len_in1, in1, in1_len, _in1);
   copy(len_in2, in2, in2_len, _in2);

   outer = am1;
   relnName = new char [strlen(relationName.c_str())+1];
   strncpy(relnName, relationName.c_str(), strlen(relationName.c_str())+1);

   inName = new char [strlen(index_name.c_str())+1];
   strncpy(inName, index_name.c_str(), strlen(index_name.c_str())+1);

   OutputFilter = outFilter;
   RightFilter  = rightFilter;

   n_buf_pgs    = amt_of_mem;
//   inner = NULL;      // BK
   done  = FALSE;
   get_from_outer = TRUE;

   AttrType *Jtypes;
   short    *t_size;

   inner_tuple = (Tuple *) new char [Tuple::max_size];
   inner_tuple->setHdr(len_in2, in2, t2_str_sizes);

   perm_mat = proj_list;
   nOutFlds = n_out_flds;
   setup_op_tuple(Jtuple, Jtypes,
		  in1, len_in1, in2, len_in2,
		  t1_str_sizes, t2_str_sizes, t_size,
		  proj_list, nOutFlds);

   delete [] Jtypes;
   delete [] t_size;

   // open heap file for random access.
   hf = new HeapFile(relnName, s);
   index_only = indexOnly;
}


index_nested_loop::~index_nested_loop()
{
   delete [] (char *) Jtuple;
   delete [] relnName;
   delete [] inName;
   delete [] _in1;
   delete [] _in2;
   if (Index != NULL)	// If this not the first time,
      delete Index;	// delete index
   if (Iscan != NULL)	// If this not the first time,
      delete Iscan;	// close scan
   delete [] inner_tuple;
   delete hf;
}


//
// Performs the get next tuple function.
//

Status index_nested_loop::get_next(Tuple * & tuple)
{
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
	 if (Index != NULL)	// If this not the first time,
	    delete Index;	// delete index
	 if (Iscan != NULL)	// If this not the first time,
	    delete Iscan;	// close scan

	 // Open a scan on the relnName -- TO BE IMPLEMENTED
#ifdef TEST
	 //error("The inner has not been opened -- will lead to an error");
#else
	 switch (inType)
	 {
	  case B_Index:
	    Index = new BTreeFile(s, inName);	// Check s for errors
	    // BK => was assert(Iscan);
	    assert(Index);
	    
	    // IndexUtiles decides which scan to create
	    IndexUtils utils;
	    Iscan = (BTreeFileScan*)utils.BTree_scan(RightFilter, Index);
	    break;
	  case Hash:
	    // Check to make sure that this is the case...

/*
            if (!RightFilter[0])
	    {
	      std::cerr << "Attempt to use Linear Hash index to scan entire relation!\n";
	      std::cerr << "Impossible at this moment\n";
	      return DONE;
        }

	    AttrType temp_hash;
	    if (RightFilter[0]->type1 == attrSymbol)
	      {
		temp_hash = RightFilter[0]->type2;
		if (RightFilter[0]->type1 == attrSymbol)
		  std::cerr << "Invalid 2...";
	      }
	    else
	      if (RightFilter[0]->type1 == attrSymbol)
		temp_hash = RightFilter[0]->type1;
	      else
		std::cerr << "Invalid 1...";

//	    Index = new (LinearHashingFile*)LinearHashingFile(s, inName);	// check s for errors
	    Index = new LinearHashingFile(s, inName);      // checks for errors
	    // BK => was assert(Iscan);
	    assert(Index);
	    
	    switch (temp_hash) {
	      
	    case attrInteger:
	      Iscan = ((LinearHashingFile*)Index)->new_scan(
					 (const void*)&RightFilter[0]->operand1.integer);
	      break;
	      
	    case attrReal:
	      Iscan = ((LinearHashingFile*)Index)->new_scan(	      
					 (const void*)&RightFilter[0]->operand1.real);
	      break;
	    case attrString:
	      Iscan = ((LinearHashingFile*)Index)->new_scan(
					 (const void*)RightFilter[0]->operand1.string);
	      break;
	    case attrSymbol:
	      std::cerr << "Error in index-nl.C\n";
	      break;
	    case attrNull:
	    default:
	      std::cerr << "Not supposed to get here..\n";
	      MINIBASE_FIRST_ERROR( JOINS, JoinError::BAD_SCAN );
	      break;
	      
	    }
*/
	    break;
	  default:
	    std::cerr<<"INL - index type is invalid" << std::endl;
	
	 }
#endif
	 if ((s = outer->get_next(outer_tuple)) == DONE)
	 {
	    done = TRUE;
	    return DONE;
	 }
	 else if (s != OK) return s;		// return error code.
}	 
	 // Now open a scan on the index
//	 char * ptr = outer_tuple->getFld(jc1);
/*
	 // switch statement is added by BK
	 switch (inType)
	 {
	   case B_Index:
	     IndexUtils utils;
	     Iscan = (BTreeFileScan*)utils.BTree_scan(RightFilter, Index);
	     break;
           case Hash:
	     Iscan = Index->new_scan((void*)ptr);
	     break;
           defaut:
	     std::cerr << "Error in index n join\n";
	     break;
        }
*/
//	 assert(Iscan);
//      

      // ================================================================================
      //
      // The next step is to get a tuple from the inner,
      // while the inner is not completely scanned && there is no match (with pred),
      //      get a tuple from the inner.

      RID rid;
      int unused;
      void* key = new char[Iscan->keysize()];
      while ((s = Iscan->get_next(rid, key)) != DONE)
      {
	 if (s != OK) return s;

	 if (index_only)
	 {
          if (_in2[jc2] == attrInteger)
	    inner_tuple->setFld(1, *((int*)key)); 
          else
	    if (_in2[jc2] == attrReal)
	      inner_tuple->setFld(1, *((float*)key));
            else
	      if (_in2[jc2] == attrString)
		inner_tuple->setFld(1, (char*)key);
              else
	      {
		std::cout << "ERROR IN IndexNestedLoops::get_next\n";
		return DONE;
              }
         }
	 else
	 {
	   s = hf->getRecord(rid, (char*)inner_tuple, unused);
	   if (s != OK) return s;
         }

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


