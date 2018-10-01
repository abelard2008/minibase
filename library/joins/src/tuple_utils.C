
//#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include "tuple.h"

#include "tuple_utils.h"

// ========================================================================
//
// This function (surprisingly) compares a tuple with another tuple, and
// returns:
//
// 	0	if the two are equal
//	1	if the tuple is greater
//     -1	if the tuple is smaller
//
// Parameters:
//	fldType	the type of the field being compared.
//	t1	one tuple.
//	t2	another tuple.
//	t1_fld_no, t2_fld_no	the field numbers in the tuples to be \
//				compared.
//

int CompareTupleWithTuple(AttrType fldType,
			  Tuple * t1, int t1_fld_no,
			  Tuple * t2, int t2_fld_no)
{
   int   t1_i,  t2_i;
   float t1_r,  t2_r;
   char *t1_s, *t2_s;

   switch (fldType)
   {
    case attrInteger:		// Compare two integers.
      t1->getFld(t1_fld_no, t1_i);
      t2->getFld(t2_fld_no, t2_i);
      if (t1_i == t2_i) return  0;
      if (t1_i <  t2_i) return -1;
      if (t1_i >  t2_i) return  1;

    case attrReal:		// Compare two floats
      t1->getFld(t1_fld_no, t1_r);
      t2->getFld(t2_fld_no, t2_r);
      if (t1_r == t2_r) return  0;
      if (t1_r <  t2_r) return -1;
      if (t1_r >  t2_r) return  1;

    case attrString:		// Compare two strings
      t1->getFld(t1_fld_no, t1_s);
      t2->getFld(t2_fld_no, t2_s);
      // Now handle the special case that is posed by the max_values for strings...
      if (t2_s[0] == '\0') return 1;	// Smallest string
      if (t2_s[0] == char(127) && t2_s[1] == '\0') return -1;  // Largest string.
      return strcmp(t1_s, t2_s);
    default:
      /*error("Don't know what to do with attrSymbol, attrNull - CompareTupleWithTuple(...)"); */
	return 0;  // added by Ranjani
   }
   return 0;	// Control can never reach here -- inserted to stop compiler from giving a warning.
}

int CompareTupleWithValue(AttrType fldType,
			  Tuple * t1, int t1_fld_no,
			  Tuple * value)
{
   return CompareTupleWithTuple(fldType, t1, t1_fld_no, value, 1);
}



char * Value(Tuple * tuple, int fldno)
{
   return tuple->getFld(fldno);
}


int Number(AttrType fldType, Tuple *tuple, int colNo)
{
   int   temp_i = 0;
   float temp_r;
   char *temp_s;

   switch (fldType)
   {
    case attrInteger:
      return tuple->getFld(colNo, temp_i);
    case attrReal:
      return int(tuple->getFld(colNo, temp_r));
    case attrString:
      tuple->getFld(colNo, temp_s);
      assert(temp_s);
      while (*temp_s != '\0')
      {
         temp_i = *temp_s;
         temp_s++;
      }
      return temp_i;
    default:
      //error("Don't know what to do with attrSymbol, attrNull - Number(...)");
      printf("error here..\n");
      return 0; //added by Ranjani
   }
   return 0;	// Control can never reach here -- inserted to stop compiler from giving a warning.
}


void SetValue(Tuple *& value, Tuple *& tuple, int fld_no, int fldType)
{
   int   temp_i;
   float temp_f;
   char *temp_s;

   switch (fldType)
   {
    case attrInteger:
      value->setFld(1, tuple->getFld(fld_no, temp_i));
      break;
    case attrReal:
      value->setFld(1, tuple->getFld(fld_no, temp_f));
      break;
    case attrString:
      value->setFld(1, tuple->getFld(fld_no, temp_s));
      break;
    default:
      //error("Don't know how to handle attrSymbol, attrNull - SetValue(...)");
      break;
   }

   return;
}



void copy(int len, AttrType in[], int & out_len, AttrType * & _in)
{
   out_len = len;
   _in = new AttrType[len];
   for (int i = 0; i < len; i++)
      _in[i] = in[i];
}



void setup_op_tuple(Tuple * &Jtuple, AttrType * &res_attrs,
                    AttrType in1[], int len_in1, AttrType in2[], int len_in2,
            short *t1_str_sizes, short *t2_str_sizes, short *& res_str_sizes,
		    FldSpec * proj_list, int nOutFlds)
{
   short * sizesT1 = new short [len_in1];
   short * sizesT2 = new short [len_in2];
   int i, count = 0;

   for (i = 0; i < len_in1; i++)
      if (in1[i] == attrString)
	 sizesT1[i] = t1_str_sizes[count++];

   for (count = 0, i = 0; i < len_in2; i++)
      if (in2[i] == attrString)
	 sizesT2[i] = t2_str_sizes[count++];

   int n_strs = 0;
   res_attrs  = new AttrType [nOutFlds];

   for (i = 0; i < nOutFlds; i++)
   {
      if (proj_list[i].relation == outer)
	 res_attrs[i] = in1[proj_list[i].offset-1];
      else if (proj_list[i].relation == innerRel)
	 res_attrs[i] = in2[proj_list[i].offset-1];
   }

   // Now construct the res_str_sizes array.
   for (i = 0; i < nOutFlds; i++)
   {
      if (proj_list[i].relation == outer && in1[proj_list[i].offset-1] == attrString)
	 n_strs++;
      else if (proj_list[i].relation == innerRel && in2[proj_list[i].offset-1] == attrString)
	 n_strs++;
   }

   res_str_sizes = new short [n_strs];
   count         = 0;
   for (i = 0; i < nOutFlds; i++)
   {
      if (proj_list[i].relation == outer && in1[proj_list[i].offset-1] == attrString)
	 res_str_sizes[count++] = sizesT1[proj_list[i].offset-1];
      else if (proj_list[i].relation == innerRel && in2[proj_list[i].offset-1] == attrString)
	 res_str_sizes[count++] = sizesT2[proj_list[i].offset-1];
   }
   Jtuple = (Tuple *) new char [Tuple::max_size];
   Jtuple->setHdr(nOutFlds, res_attrs, res_str_sizes);

   delete [] sizesT1;
   delete [] sizesT2;
}




/*************************************
  setup_op_tuple
**************************************

Jtuple - ptr to an actual tuple ???? - no memory has been malloced
res_attrs - attributes of resultant tuple
in1 - array of the attributes of the tuple (ok)
len_in1 - num of attributes
t1_str_sizes - 
*/
void setup_op_tuple(Tuple * &Jtuple, AttrType * &res_attrs,
                    AttrType in1[], int len_in1,
		    short *t1_str_sizes,  short *& res_str_sizes,
		    FldSpec * proj_list, int nOutFlds)
{
   short * sizesT1 = new short [len_in1];
   int i, count = 0;

   for (i = 0; i < len_in1; i++)
      if (in1[i] == attrString)
	 sizesT1[i] = t1_str_sizes[count++];

   int n_strs = 0;
   res_attrs  = new AttrType [nOutFlds];

   for (i = 0; i < nOutFlds; i++)
   {
      if (proj_list[i].relation == outer)
	 res_attrs[i] = in1[proj_list[i].offset-1];
      else if (proj_list[i].relation == innerRel){
	assert(0);    
      }
   }

   // Now construct the res_str_sizes array.
   for (i = 0; i < nOutFlds; i++)
   {
      if (proj_list[i].relation == outer && in1[proj_list[i].offset-1] == attrString)
	 n_strs++;
   }

   res_str_sizes = new short [n_strs];
   count         = 0;
   for (i = 0; i < nOutFlds; i++)
   {
      if (proj_list[i].relation == outer && in1[proj_list[i].offset-1] == attrString)
	 res_str_sizes[count++] = sizesT1[proj_list[i].offset-1];
   }
   Jtuple = (Tuple *) new char [Tuple::max_size];
   Jtuple->setHdr(nOutFlds, res_attrs, res_str_sizes);

   delete [] sizesT1;

}



int Equal(Tuple *t1, Tuple *t2, AttrType types[], int len)
{
   int i;

   for (i = 1; i <= len; i++)
      if (CompareTupleWithTuple(types[i-1], t1, i, t2, i) != 0)
	 return FALSE;
   return TRUE;
}



