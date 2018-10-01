#ifndef __FILE_SCAN__
#define __FILE_SCAN__

#include "iterator.h"
#include "heapfile.h"
#include "scan.h"
#include "tuple_utils.h"
#include "projection.h"
#include "pred_eval.h"
#include <string>

class FileScanIter : public Iterator
{
private:
  AttrType *_in1;
  int in1_len;
  HeapFile *f;
  Scan *scan;
  Tuple    * tuple1;
  Tuple    *Jtuple;
  int        t1_size;
  int nOutFlds;
//  FldSpec *perm_mat;
  CondExpr * * OutputFilter;

public:
FldSpec *perm_mat;


 FileScanIter(std::string file_name,
		  AttrType in1[],		// Array containing field types of tups
		  short s1_sizes[], 
		  int     len_in1,		// # of columns in tups
          int n_out_flds,
          FldSpec *proj_list,
		  CondExpr ** outFilter,	// Ptr to the output filter
		  Status & s
		  );

FldSpec* show()
{
 return perm_mat;
}


  Status get_next(Tuple * &tuple);		// The tuple is returned.
  ~FileScanIter();
};


#endif
