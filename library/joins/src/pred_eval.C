//#include "test.h"
#include "tuple.h"
#include "iterator.h"
#include "pred_eval.h"
#include "tuple_utils.h"

int Eval(CondExpr *p[], Tuple *t1, Tuple *t2, AttrType in1[], AttrType in2[])
{
   CondExpr *temp_ptr;
   int       i = 0;

   Tuple    *tuple1 = 0, *tuple2 = 0;
   int       fld1, fld2;
   Tuple    *value = (Tuple *) new char [Tuple::max_size];

   short     str_size[1];
   AttrType  val_type[1];

   AttrType  comparison_type = attrInteger;
   int       comp_res;
   int      op_res = FALSE, row_res = FALSE, col_res = TRUE;

   if (p == NULL)
   {
      delete [] value;  // added by BK
      return TRUE;
   }

   while (p[i] != NULL)
   {
      temp_ptr = p[i];
      while (temp_ptr != NULL)
      {
	 val_type[0] = temp_ptr->type1;
	 fld1        = 1;
	 switch (temp_ptr->type1)
	 {
	  case attrInteger:
	    value->setHdr(1, val_type, NULL);
	    value->setFld(1, temp_ptr->operand1.integer);
	    tuple1 = value;
	    comparison_type = attrInteger;
	    break;
	  case attrReal:
	    value->setHdr(1, val_type, NULL);
	    value->setFld(1, temp_ptr->operand1.real);
	    tuple1 = value;
	    comparison_type = attrReal; 
	    break;
	  case attrString:
	    str_size[0] = strlen(temp_ptr->operand1.string) + 1;
	    value->setHdr(1, val_type, str_size);
	    value->setFld(1, temp_ptr->operand1.string);
	    tuple1 = value;
	    comparison_type = attrString;
	    break;
	  case attrSymbol:
	    fld1 = temp_ptr->operand1.symbol.offset;
	    if (temp_ptr->operand1.symbol.relation == outer)
	    {
	       tuple1 = t1;
	       comparison_type = in1[fld1-1];;
	    }
	    else
	    {
	       tuple1 = t2;
	       comparison_type = in2[fld1-1];
	    }
	    break;
	  default:
	    break;
	 }

	 // Setup second argument for comparison.
	 val_type[0] = temp_ptr->type2;
	 fld2        = 1;
	 switch (temp_ptr->type2)
	 {
	  case attrInteger:
	    value->setHdr(1, val_type, NULL);
	    value->setFld(1, temp_ptr->operand2.integer);
	    tuple2 = value;
	    break;
	  case attrReal:
	    value->setHdr(1, val_type, NULL);
	    value->setFld(1, temp_ptr->operand2.real);
	    tuple2 = value;
	    break;
	  case attrString:
	    str_size[0] = strlen(temp_ptr->operand2.string) + 1;
	    value->setHdr(1, val_type, str_size);
	    value->setFld(1, temp_ptr->operand2.string);
	    tuple2 = value;
	    break;
	  case attrSymbol:
	    fld2 = temp_ptr->operand2.symbol.offset;
	    if (temp_ptr->operand2.symbol.relation == outer)
	       tuple2 = t1;
	    else
	       tuple2 = t2;
	    break;
	  default:
	    break;
	 }


	 // Got the arguments, now perform a comparison.
	 comp_res = CompareTupleWithTuple(comparison_type, tuple1, fld1, tuple2, fld2);
	 op_res = FALSE;

	 switch (temp_ptr->op)
	 {
	  case aopEQ:
	    if (comp_res == 0) op_res = TRUE;
	    break;
	  case aopLT:
	    if (comp_res <  0) op_res = TRUE;
	    break;
	  case aopGT:
	    if (comp_res >  0) op_res = TRUE;
	    break;
	  case aopNE:
	    if (comp_res != 0) op_res = TRUE;
	    break;
	  case aopLE:
	    if (comp_res <= 0) op_res = TRUE;
	    break;
	  case aopGE:
	    if (comp_res >= 0) op_res = TRUE;
	    break;
	  case aopNOT:
	    if (comp_res != 0) op_res = TRUE;
	    break;
	  default:
	    break;
	 }

	 row_res = row_res || op_res;
	 if (row_res == TRUE)
	    break;			// OR predicates satisfied.
	 temp_ptr = temp_ptr->next;
      }
      i++;

      col_res = col_res && row_res;
      if (col_res == FALSE)
      {
	 delete [] value; // added by BK
	 return FALSE;
      }
      row_res = FALSE;			// Starting next row.
   }
   
   delete [] value;
   return TRUE;
}


