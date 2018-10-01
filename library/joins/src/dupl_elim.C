// =============================================================================
//
// dupl_elim.C
//	This file contains the implementation of a class that eliminates
// duplicates from a stream of tuples. It basically uses a sort utility to
// sort the tuples and then eliminated dulpicate tuples.
//

#include <stdio.h>
#include <fcntl.h>
#include "iterator.h"
//#include "test.h"
#include "tuple.h"
#include "sort.h"
#include "tuple_utils.h"
#include "projection.h"
#include "pred_eval.h"
#include "dupl_elim.h"

DuplElim::DuplElim(
		   AttrType in[],		// Array containing field types of R.
		   int      len_in,		// # of columns in R.
		   short    s_sizes[],
		   
		   Iterator *am,	// access method for left input to join.
		   int      amt_of_mem,	// IN PAGES
		   
		   int     inp_sorted,
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
   copy(len_in, in, in_len, _in);

   Jtuple = (Tuple *) new char [Tuple::max_size];
   Jtuple->setHdr(len_in, in, s_sizes);

   sortFldType = in[0];
   switch (sortFldType)
   {
    case attrInteger:
      sortFldLen = sizeof(int);
      break;
    case attrReal:
      sortFldLen = sizeof(float);
      break;
    case attrString:
      sortFldLen = s_sizes[0];
      break;
    default:
      //error("Unknown type");
      return;
   }
   _am = am;
   if (!inp_sorted)
   {
      _am = new Sort(in, len_in, s_sizes, am, 1, Ascending,
		     sortFldLen, amt_of_mem, s);

      if (s != OK) return;
   }

   Tuple *t;
   // Allocate memory for the temporary tuples
   TempTuple1 = (Tuple *) new char [Tuple::max_size];
   TempTuple2 = (Tuple *) new char [Tuple::max_size];

   TempTuple1->setHdr(in_len, _in, s_sizes);
   TempTuple2->setHdr(in_len, _in, s_sizes);

   done = FALSE;

   // Now, that stuff is setup, all we have to do is a get_next !!!!
   s = _am->get_next(t);
   memcpy((char *) TempTuple1, (char *) t, TempTuple1->size());
}


DuplElim::~DuplElim()
{
   delete [] (char *) TempTuple1;
   delete [] (char *) TempTuple2;
   delete [] (char *) Jtuple;

   // delete children
   delete _am;
}



// The current tuple is to be returned, scan to get next tuple.

Status DuplElim::get_next(Tuple * &tuple)
{
   Status s = OK;
   Tuple * t;

   if (done) return DONE;

   memcpy((char *) Jtuple, (char *) TempTuple1, Jtuple->size());
   do
   {
      if ((s = _am->get_next(t)) == DONE)
      {
	 done = TRUE;			// next call returns DONE;
	 return DONE;
      }
      else if (s != OK) return s;	// return error code
      memcpy((char *) TempTuple2, (char *) t, TempTuple1->size());
   }
   while (Equal(TempTuple1, TempTuple2, _in, in_len));

   // Now copy the the TempTuple2 (new o/p tuple) into TempTuple1.
   memcpy((char *) TempTuple1, (char *) TempTuple2, Jtuple->size());
   memcpy((char *) Jtuple,     (char *) TempTuple2, Jtuple->size());
   tuple = Jtuple;
   return OK;
}


// The current tuple is to be returned, scan to get next tuple.

/*
Status DuplElim::get_next(Tuple * &tuple)
{
   Status s = OK;
   Tuple * t;

   if (done) return DONE;

   memcpy((char *) Jtuple, (char *) TempTuple1, Jtuple->size());
   do
   {
      if ((s = _am->get_next(t)) == DONE)
      {
	 done = TRUE;			// next call returns DONE;
	 return DONE;
      }
      else if (s != OK) return s;	// return error code
      memcpy((char *) TempTuple2, (char *) t, TempTuple1->size());
   }
   while (Equal(TempTuple1, TempTuple2, _in, in_len));

   // Now copy the the TempTuple2 (new o/p tuple) into TempTuple1.
   memcpy((char *) TempTuple1, (char *) TempTuple2, Jtuple->size());
   memcpy((char *) Jtuple,     (char *) TempTuple2, Jtuple->size());
   tuple = Jtuple;
   return OK;
}

*/
