// Open the scans for BTree...
// Requires one or two: selects[0] & selects[1]
// 

// Check the types...
// Check that both are not symbol => one each...

// DOES NOT HANDLE "Not" queries...

#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
//#include "test.h"
#include "tuple.h"
#include "iterator.h"
#include "projection.h"
#include "tuple_utils.h"          // Unneeded
#include "pred_eval.h"            // Unneded 

#include "index_scan.h"
#include "index.h"
#include "btfile.h"

IndexFileScan* IndexUtils::BTree_scan(CondExpr* selects[], IndexFile* indFile)
{

  IndexFileScan *indScan;

  if (selects[0] == NULL)
    {
      indScan = ((BTreeFile*)indFile)->new_scan();
      return indScan;
    }



  if (selects[1] == NULL)
    {
      if (!(selects[0]->type1 == attrSymbol) && !(selects[0]->type2 == attrSymbol))
	{
	  std::cerr << "Invalid selection...\n";
	  return NULL;
	}
     
      // SYMBOL = VALUE
      if (selects[0]->op == aopEQ)
	{
	  void * ptr;
          
	  if (selects[0]->type1 != attrSymbol)
	  {
	      ptr = getValue(selects[0], selects[0]->type1, 1);
	      indScan = ((BTreeFile*)indFile)->new_scan(ptr, ptr);
          }
	  else
	  {
	      ptr = getValue(selects[0], selects[0]->type2, 2);
	      indScan = ((BTreeFile*)indFile)->new_scan(ptr, ptr);
          }
	  return indScan;
	}

      // SYMBOL < VALUE   or   SYMBOL <= VALUE
      if ((selects[0]->op == aopLT) || (selects[0]->op == aopLE))
	{
	  if (selects[0]->type1 != attrSymbol)
	    indScan = ((BTreeFile*)indFile)->new_scan(NULL, &selects[0]->operand1);
	  else
	    indScan = ((BTreeFile*)indFile)->new_scan(NULL, &selects[0]->operand2);
	  return indScan;
	}


      if ((selects[0]->op == aopGT) || (selects[0]->op == aopGE))
	{
	  if (selects[0]->type1 != attrSymbol)
	    indScan = ((BTreeFile*)indFile)->new_scan(&selects[0]->operand1, NULL);
	  else
	    indScan = ((BTreeFile*)indFile)->new_scan(&selects[0]->operand2, NULL);
	  return indScan;
	}


      std::cerr << "ERROR -- in IndexScanIter\n";
      return NULL;
    }
  
  // MUST BE A RANGE QUERY....
  else
    {
      if (!(selects[0]->type1 == attrSymbol) && !(selects[0]->type2 == attrSymbol))
	{
	  std::cerr << "Invalid selection...\n";
	  return NULL;
	}

      if (!(selects[1]->type1 == attrSymbol) && !(selects[1]->type2 == attrSymbol))
	{
	  std::cerr << "Invalid selection...\n";
	  return NULL;
	}
      

      // WHICH SYMBOL IS HIGHER???
      void * ptr1;
      void * ptr2;
      AttrType type;
      
      if (selects[0]->type1 != attrSymbol)
	{
	  ptr1 = getValue(selects[0], selects[0]->type1, 1);
	  type = selects[0]->type1;
	}
      else
	{
	  ptr1 = getValue(selects[0], selects[0]->type2, 2);
	  type = selects[0]->type2;
	}
      if (selects[1]->type1 != attrSymbol)
	ptr2 = getValue(selects[1], selects[1]->type1, 1);
      else
	ptr2 = getValue(selects[1], selects[1]->type2, 2);

      if (type == attrString)
	{
          if (strcmp((const char*)ptr1, (const char*)ptr2))
	    indScan = ((BTreeFile*)indFile)->new_scan(ptr1, ptr2);
          else
	    indScan = ((BTreeFile*)indFile)->new_scan(ptr2, ptr1);
	}
      else
	{
	  if (type == attrInteger)
	    if (*(int*)ptr1 < *(int*)ptr2)
	      indScan = ((BTreeFile*)indFile)->new_scan(ptr1, ptr2);
	    else
	      indScan = ((BTreeFile*)indFile)->new_scan(ptr2, ptr1);
	  else
	    if (type == attrReal)
	      if (*(float*)ptr1 < *(float*)ptr2)
		indScan = ((BTreeFile*)indFile)->new_scan(ptr1, ptr2);
	      else
		indScan = ((BTreeFile*)indFile)->new_scan(ptr2, ptr1);
	    else
	      {
		std::cerr << "Error!!!! - index_scanIter\n";
		return NULL;
	      }
	  return indScan;
	}

    }  // closes else
  return indScan;
}

void* IndexUtils::getValue(CondExpr* cd, AttrType type, int choice)
{

void * ptr;
// Error checking.......
  if (cd == NULL)
    return NULL;
  if ((choice < 1) || (choice > 2))
    return NULL;

 switch (type)
 {
   case attrString:
     if (choice == 1)
       ptr = (void*)cd->operand1.string;
     else
       ptr = (void*)cd->operand2.string;
     break;

   case attrInteger:
     if (choice == 1)
       ptr = &cd->operand1.integer;
     else
       ptr = &cd->operand2.integer;
     break;

   case attrReal:
     if (choice == 1)
       ptr = &cd->operand1.real;
     else
       ptr = &cd->operand2.real;
     break;

   default:
     std::cerr << "error....";
     std::cerr << "AttrType = " << type << std::endl;
     return NULL;
     break;
};

  return ptr;
}


