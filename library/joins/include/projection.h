#ifndef __PROJECTION__
#define __PROJECTION__

//#include "test.h"
#include "tuple.h"
#include "iterator.h"

// Jtuple has the appropriate types.

void Join(Tuple * t1, AttrType type1[], int n_types1,
	  Tuple * t2, AttrType type2[], int n_types2,
	  Tuple *&Jtuple, FldSpec * perm_mat, int nOutFlds);

void Project(Tuple * t1, AttrType type1[], int n_types1,
          Tuple *&Jtuple, FldSpec * perm_mat, int nOutFlds);


#endif
