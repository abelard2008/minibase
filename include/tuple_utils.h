#ifndef __TUPLE_UTILS__
#define __TUPLE_UTILS__

//#include "test.h"
#include "tuple.h"
#include "iterator.h"

class FldSpec;

int CompareTupleWithTuple(AttrType fldType,
			  Tuple * t1, int t1_fld_no,
			  Tuple * t2, int t2_fld_no);

int CompareTupleWithValue(AttrType fldType,
			  Tuple * t1, int t1_fld_no,
			  Tuple * value);

int Equal(Tuple *t1, Tuple *t2, AttrType types[], int len);

char * Value(Tuple * tuple, int fldno);
int Number(AttrType fldType, Tuple *tuple, int colNo);
void SetValue(Tuple *& value, Tuple *& tuple, int fld_no, int fldType);

void copy(int len, AttrType in[], int & out_len, AttrType * & _in);
void setup_op_tuple(Tuple * &Jtuple, AttrType * &res_attrs,
		    AttrType in1[], int len_in1, AttrType in2[], int len_in2,
	     short *t1_str_sizes, short *t2_str_sizes, short *& res_str_sizes,
		    FldSpec* proj_list, int nOutFlds);

void setup_op_tuple(Tuple *&Jtuple, AttrType * &res_attrs,
                    AttrType in1[], int len_in1,
                    short *t1_str_sizes, short *& res_str_sizes,
                    FldSpec* proj_list, int nOutFlds);



#endif
