#include <ostream>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
//#include "test.h"
#include "tuple.h"
#include "iterator.h"
#include "projection.h"
#include "tuple_utils.h"
#include "pred_eval.h"

#include "file_scan.h"

// BK
extern int total;

/************************************
     FileScanIter::FileScanIter
*************************************

in1[] - array showing what the attributes of the 
  input fields are.
s1_sizes[] - shows the length of the string fields.
len_in1 - number of attributes in the input tuple
n_out_flds - number of fields in the out tuple
proj_list - shows what input fields go where in the
  output tuple
outFilter - select expressions
*/
FileScanIter::FileScanIter  ( std::string file_name,
		       AttrType in1[],
		       short s1_sizes[], 
		       int     len_in1,
                       int n_out_flds,
                       FldSpec *proj_list,
		       CondExpr ** outFilter, 
		       Status & s
		       )
{
  
   _in1    = new AttrType[len_in1];
   in1_len = len_in1;
   for (int i = 0; i < len_in1; i++)
      _in1[i] = in1[i];

//   Jtuple = (Tuple *) new char [Tuple::max_size];
//   Jtuple->setHdr(len_in1, _in1, s1_sizes);

   // Compute Jtypes
   AttrType *Jtypes;
   short    *ts_size;

   setup_op_tuple(Jtuple, Jtypes, in1, len_in1, s1_sizes, ts_size, proj_list, n_out_flds);
   OutputFilter = outFilter;
   perm_mat = proj_list;
   nOutFlds = n_out_flds; 
   tuple1 = (Tuple *) new char [Tuple::max_size];
   tuple1->setHdr(in1_len, _in1, s1_sizes);
   t1_size = tuple1->size();

   f = new HeapFile(file_name, s);
   if(s != OK) return;  // std::cout<< "Error creating a Heapfile"<<endl;

   scan = f->openScan(s);
  
   if (s != OK) return; // std::cout <<"Error opening scan"<<endl, assert(0);

}

Status FileScanIter::get_next(Tuple * &tuple)
{
   Status s;
   RID rid;
   int len;

   while(1) {
      if((s = scan->getNext(rid, (char *)tuple1, len)) == DONE) {
	 tuple = NULL;
	 return DONE;
      }
      else if (s != OK) {
	 tuple = NULL;
	 return DONE;
      }
/*
      Project(tuple1, _in1, in1_len, Jtuple, perm_mat, nOutFlds);

      if(Eval(OutputFilter, Jtuple, NULL, _in1, NULL) == TRUE){
*/
      if (Eval(OutputFilter, tuple1, NULL, _in1, NULL) == TRUE){
	 Project(tuple1, _in1, in1_len, Jtuple, perm_mat, nOutFlds); 
	 tuple = Jtuple;
	 return OK;
      }	
   }
}        

    
FileScanIter::~FileScanIter()
{
  delete [] (char *) Jtuple;
  delete [] (char *) tuple1;
  delete f; 
  delete scan;
  delete _in1;   // added by BK  => should be delete [] _in1?????
}
