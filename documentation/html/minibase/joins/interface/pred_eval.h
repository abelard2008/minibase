#ifndef __PRED_EVAL__
#define __PRED_EVAL__

#include "tuple.h"
#include "iterator.h"

int Eval(CondExpr *p[], Tuple *t1, Tuple *t2, AttrType in1[], AttrType in2[]);

#endif
