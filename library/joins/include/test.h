#ifndef __TEST__ 
#define __TEST__







#include "tuple.h"
#include "iterator.h"
#include "minirel.h"


#define FLOAT_MIN	1.0e-25
#define FLOAT_MAX	1.0e+25
#define bool    	int
#define page_size MINIBASE_PAGESIZE
#define T_SIZE	50

// enum IndexType {None, B_Index, Hash};

// #include "iterator.h"
/*
class FileScan : public Iterator
{
 private:
   int     upr, value;
   char  * dummy;
   Tuple * tuple;
   int     n_flds;
 public:
   FileScan(char *name, AttrType types[], int n_types);
   Status get_next(Tuple * & t);
   ~FileScan();
};
*/

void error(char * s);

#endif


