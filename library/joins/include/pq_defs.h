#ifndef __PQ_DEFS__
#define __PQ_DEFS__

//#include "test.h"
#include "tuple.h"
#include "tuple_utils.h"

struct node {
	int     run_num;
	Tuple * tuple;
};

typedef struct node * pnode;

#endif
