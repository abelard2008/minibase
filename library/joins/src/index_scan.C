/******************************************

Written by Bill Kimmel
kimmel@cs.wisc.edu

******************************************/

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

#include "join_errors.h"
#include "index_scan.h"
#include "index.h"
/*  #include "linearhashing.h"  */
#include "btfile.h"



/********************************************

NOT TRUE!!!!!
selects[] - must be on one field only: the
 key field.  This array can be only one 
 element for Hash and two for BTree.

********************************************/
IndexScanIter::IndexScanIter(
	IndexType index,
	std::string relName, 	// Name of the input relation
	std::string indName,
	AttrType types[],	// Array of types in this relation
	short* str_sizes,	// array of string sizes
	int noInFlds,		// Number of fields in input tuple 
	int noOutFlds,		// Number of fields in output tuple
	FldSpec outFlds[],	// Fields to project
	CondExpr* selects[],	// Conditions to apply
	const int fldNum,       
	const int indexOnly,
	Status& s
){

  _fldNum = fldNum;
  _types = new AttrType[noInFlds];
  _noInFlds = noInFlds;
  for (int i = 0; i < noInFlds; i++)
     _types[i] = types[i];

  AttrType *Jtypes;
  short *ts_size;

  setup_op_tuple(Jtuple, Jtypes, types, noInFlds, str_sizes, ts_size, outFlds, noOutFlds);

  _selects = selects;
  perm_mat = outFlds;
  _noOutFlds = noOutFlds;

  tuple1 = (Tuple *) new char [Tuple::max_size];
  tuple1->setHdr(noInFlds, types, str_sizes);
  t1_size = tuple1->size();

  f = new HeapFile(relName, s);
  if(s != OK) 
  {
    s = PLANNER;
    return;  // std::cout<< "Error creating a Heapfile"<<endl;
  }


  // Above code is almost identical to FileScanIter
  // Now must set up the index scan efficiently.

  switch (index) {

  // Hash selections should only be of the form:
  //    value = symbol 
  case Hash:

/*
    if (selects[0] == NULL)
    {
      std::cerr << "Attempt to use Linear Hash index to scan entire relation!\n";
      std::cerr << "Impossible at this moment\n";
      s = JOINS;
      return;
    }


    // Check to make sure that this is the case...
    AttrType temp_hash;
    if (selects[0]->type1 == attrSymbol)
    {
      temp_hash = selects[0]->type2;
      if (selects[0]->type1 == attrSymbol)
	std::cerr << "Invalid 2...";
    }
    else
      if (selects[0]->type2 == attrSymbol)
	temp_hash = selects[0]->type1;
      else
	std::cerr << "Invalid 1...";
	
    indFile = new LinearHashingFile(s, indName);

      switch (temp_hash) {

      case attrInteger:
	indScan = ((LinearHashingFile*)indFile)->new_scan(
			     (const void*)&selects[0]->operand1.integer);
	break;

      case attrReal:
	indScan = ((LinearHashingFile*)indFile)->new_scan(
			     (const void*)&selects[0]->operand1.real);
        break;
      case attrString:
	indScan = ((LinearHashingFile*)indFile)->new_scan(
			     (const void*)selects[0]->operand1.string);
        break;
      case attrSymbol:
	std::cerr << "Error in index_scan.C\n";
	break;
      case attrNull:
      default:
	std::cerr << "Not supposed to get here..\n";
	MINIBASE_FIRST_ERROR( JOINS, JoinError::BAD_SCAN );
	break;
       
    }
*/

    break;
  case B_Index:
    // Error check the select condition
    // must be of the type: value op symbol // symbol op value
    // but not symbol op symbol   // value op value


    indFile = new BTreeFile(s, indName); 

    IndexUtils utils;
    indScan = (BTreeFileScan*)utils.BTree_scan(selects, indFile);
    break;

  case None:
    s = MINIBASE_FIRST_ERROR( JOINS, JoinError::CANT_SCAN_NONE );
    return;


  default:
    s = MINIBASE_FIRST_ERROR( JOINS, JoinError::UNK_INDEX_TYPE );
    return;

  }
  index_only = indexOnly;
}



IndexScanIter::~IndexScanIter()
{
  delete f;
  delete indFile;
  delete indScan;
  delete [] _types;
  delete [] tuple1;  


}




Status IndexScanIter::get_next(Tuple*& tuple){

 RID rid;
 int unused;
 Status s;

 void* temp = new char[indScan->keysize()];
 while ((s = indScan->get_next(rid, temp)) != DONE)
 {
   if (s != OK) return s;


   // warning: _selects[0] could be NULL if it is a BTree scan...
   if (index_only)
   {
      if (_types[_fldNum - 1] == attrInteger)
	Jtuple->setFld(1, *((int*)temp));
      else
	if (_types[_fldNum - 1] == attrString)
	  Jtuple->setFld(1, (char*)temp);
        else
	  if (_types[_fldNum - 1] == attrReal)
	    Jtuple->setFld(1, *((float*)temp));
          else
	    {
	      std::cout << "ERROR IN IndexScanIter::get_next\n";
	      return DONE;
            }
      delete [] temp;
      tuple = Jtuple;
      return OK;

   }

   s = f->getRecord(rid, (char*)tuple1, unused);
   if (s != OK) return s;

   if (Eval(_selects, tuple1, NULL, _types, NULL) == TRUE)
   {
     Project(tuple1, _types, _noInFlds, Jtuple, perm_mat, _noOutFlds);
     tuple = Jtuple;
     return OK;
   }
 }
 return DONE;
}


