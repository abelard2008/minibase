// -*- mode: c++ -*-
// nestloop.c
//

#include "joinmeth.h"
#include "nestloop.h"
#include <math.h>
#include <assert.h>

// JoinFunction NestedLoopsRightSubtreeInner::JF = NLJoin_RSI;


NestedLoopsRightSubtreeInner::NestedLoopsRightSubtreeInner():JoinMethod()
{ 
  // activeJoinMethodList.Insert(this);
  name = "Tuple Oriented Nested Loops with Right SubTree as Inner"; 
  costDescription = 
    "The cost estimate is computed using the following formula for \n"
"joining plan R and plan L, where R is the right subplan and L is the \n"
"left:\n\n"
"      L.TotalCost + (L.NTuples * R.NPages) \n\n"
"where: \n"
"     Cost - the cost of accessing the relation defined by the plan \n"
"     NTuples - the number of tuples in the relation \n\n"
"     TotalCost - Cost to completely build the left plan.\n\n"
"    Note: This method does not consider indexes.\n\n"
"    The memory required for this join is three pages.  One for each of the two\n"
"    input relations and one for the output relations.\n\n";
}


void NestedLoopsRightSubtreeInner::MakePlanNode(PlanNode *pn1, PlanNode *pn2,
						const SelectList & sel, 
						Select *primaryJoinPredicate,
						const AttributeList & al,
						SolutionList *sol)
{
  PlanNode *result = NewPlanNode(pn1, pn2, sel, primaryJoinPredicate, al);
  TupleOrder tupleOrder = pn1->GetAccessMethod().Order();
  Attribute *attribute = pn1->GetAccessMethod().AttributePtr();
 
  double cost;

  cost = ((double)pn1->Cardinality() *
	  pn2->GetAccessMethod().AttributePtr()->RelationPtr()->NumPages()) + 
	  pn1->TotalCost();

  // a little bit of a hack here.  the second number should probably be
  // the number of pages returned, but it might not matter.
  result->SetAccessMethod(Plan_AccessMethod("", 0, tupleOrder, attribute, NULL));
  
  result->GetAccessMethod().SetCost(cost); 

  result->SetDescription(" (L.NTuples * R.NPages) + L.Total ");

  result->SetMemoryRequired(3);
 
  // add the result to the given solution list
  sol->Insert(result);
}

int NestedLoopsRightSubtreeInner::CanBeApplied(PlanNode *pn1, PlanNode *pn2,
						 Select *primaryJoinPredicate)
{
    if ( !pn1 || !pn2 || !pn2->RightChild()) return FALSE;
    
    pn2->children.GoHead();
    if ( pn2->children.CurrPtr()->IsPlanNode() )
	return FALSE;
    else
    {
	Plan_AccessMethod *am = pn2->children.CurrPtr()->GetPlan_AccessMethod();
	if ( am && !am->IndexPtr() && am->Order() == Random
	    && ( !primaryJoinPredicate
		|| primaryJoinPredicate->Contains(am->AttributePtr())))
	{	    
	    return TRUE;
	}
	else
	{ 
	    return FALSE;
	}    
    }    
}


PageNestedLoops::PageNestedLoops():JoinMethod()
{ 
  // activeJoinMethodList.Insert(this);
  name = "Page Oriented Nested Loops with Right SubTree as Inner"; 
  costDescription = 
    "The cost estimate is computed using the following formula for \n"
"joining plan R and plan L, where R is the right subplan and L is the \n"
"left:\n\n"
"      L.TotalCost + (L.NPages * R.NPages)\n\n"
"where:\n"
"     Cost - the cost of accessing the relation defined by the plan \n"
"     NPages - the number of pages for either L or R.\n\n"
"     TotalCost - Cost to completely build the left plan.\n\n"
"    Note: This method does not consider indexes.\n\n"
"    The memory required for this join is three pages.  One for each of the two\n"
"    input relations and one for the output relations.\n\n";
}


void PageNestedLoops::MakePlanNode(PlanNode *pn1, PlanNode *pn2,
						const SelectList & sel, 
						Select *primaryJoinPredicate,
						const AttributeList & al,
						SolutionList *sol)
{
  PlanNode *result = NewPlanNode(pn1, pn2, sel, primaryJoinPredicate, al);
  TupleOrder tupleOrder = pn1->GetAccessMethod().Order();
  Attribute *attribute = pn1->GetAccessMethod().AttributePtr();
 
  double cost;
  pn2->children.GoHead();
  Plan_AccessMethod *am = pn2->children.CurrPtr()->GetPlan_AccessMethod();

  cost = (pn1->NumOfPages() *
	  am->AttributePtr()->RelationPtr()->NumPages()) + 
	  pn1->TotalCost();

  // a little bit of a hack here.  the second number should probably be
  // the number of pages returned, but it might not matter.
  result->SetAccessMethod(Plan_AccessMethod("", result->NumOfPages(), 
					    tupleOrder, attribute, NULL));
  
  result->GetAccessMethod().SetCost(cost); 

  result->SetDescription(" (L.NPages * R.NPages) + L.Total ");

  result->SetMemoryRequired(3);
 
  // add the result to the given solution list
  sol->Insert(result);
}

int PageNestedLoops::CanBeApplied(PlanNode *pn1, PlanNode *pn2,
						 Select *primaryJoinPredicate)
{
    if ( !pn1 || !pn2 || !pn2->RightChild()) return FALSE;
    
    pn2->children.GoHead();
    if ( pn2->children.CurrPtr()->IsPlanNode() )
	return FALSE;
    else
    {
	Plan_AccessMethod *am = pn2->children.CurrPtr()->GetPlan_AccessMethod();
	if ( am && !am->IndexPtr() && am->Order() == Random
	    && (!primaryJoinPredicate
		|| primaryJoinPredicate->Contains(am->AttributePtr())))
	{	    
	    return TRUE;
	}
	else
	{ 
	    return FALSE;
	}    
    }
}


    
IndexNestedLoops::IndexNestedLoops():JoinMethod()
{ 
  // activeJoinMethodList.Insert(this);
  name = "Index Nested Loops with Right SubTree as Inner"; 
  costDescription = 
    "The cost estimate is computed using the following formula for \n"
"joining plan R and plan L, where R is the right subplan and L is the \n"
"left:\n\n"
"      L.TotalCost + (L.NTuples * R.Access)\n\n"
"where:\n"
"     Access - the cost of accessing the relation defined by the plan\n"
"     NTuples - the number of tuples in the relation\n\n"
"     TotalCost - Cost to completely build the left plan.\n\n"
"     R.Access is the cost to access the right relation using the right index.\n"
"     How much this is can vary depending on the index, and also if it is\n"
"     necessary to go out to the table or can be an index only scan.\n\n"
"    One thing about R.Access is that it will usually contain L.NTuples dividing\n"
"    the primary selectivity.  This is because we're interested in the\n"
"    selectivity of one tuple of L, not all of L.   It sometimes means\n"
"    that the cost formula will not precisely match the above formula,\n"
"    but it can always be converted to a formula in that form.\n\n"
"    The memory required for this join is three pages.  One for each of the two\n"
"    input relations and one for the output relations.\n\n";
}


void IndexNestedLoops::MakePlanNode(PlanNode *pn1, PlanNode *pn2,
						const SelectList & sel, 
						SelectList& primaryPreds,
						const AttributeList & al,
						SolutionList *sol)
{

  // There may be more than one primary predicate.
  // We will assume that they are all independent so that the total
  // selectivity is product of all singe selectivities.

  primaryPreds.GoHead();
  Select* currentPred;
  double idx_selectivity = 1;
  while((currentPred = primaryPreds.CurrPtr())){
       idx_selectivity *= currentPred->GetSelectivity();
       primaryPreds.GoNext();
  }
  PlanNode *result = new PlanNode(this, pn1, pn2, sel, primaryPreds, al);
  assert(result);

  TupleOrder tupleOrder = pn1->GetAccessMethod().Order();
  Attribute *attribute = pn1->GetAccessMethod().AttributePtr();
 
  double cost;

  pn2->children.GoHead();  
  Plan_AccessMethod* am = pn2->children.CurrPtr()->GetPlan_AccessMethod();
  
  std::string desc;
  
  if (!am && !am->IndexPtr() && (am->Order() == Random ))
  {      
      desc = " (L.NTuples * R.NPages) + L.Total ";
      cost = ( pn1->Cardinality() * pn2->NumOfPages() ) + pn1->TotalCost();
  } 
  else if ( !am->IndexPtr() )
  {      
      // desc = " (L.NTuples * log(R.NPages) + (R.NPages * RF) + L.Total ";
      desc = " L.Total + L.NTuples * ( log (R.NPages) + R.NPages * RF / L.NTuples) ";

      cost = (pn1->Cardinality() * log(pn2->NumOfPages())/log(2)) 
	      + (pn2->NumOfPages() * idx_selectivity)
	      + pn1->TotalCost();      
  }  
  else if (am->IndexPtr()->GetSelectType() == selExact && result->IndexOnly())
  {     
      // desc = " (L.NTuples * R.IHeight) + L.Total ";
      desc = " L.Total + ( R.IPages) " ;
      // cost = (am->IndexPtr()->NumPages() * pn1->Cardinality())
	//     + pn1->TotalCost();
      cost = am->NumLeafPages() + pn1->TotalCost();
  } 
  else if (am->IndexPtr()->GetSelectType() == selExact
	   && am->IndexPtr()->IsClustered())
  {     
      // desc = " (R.NPages * RF) + (L.NTuples * R.IHeight) + L.Total ";
      desc = " L.Total + L.NTuples  * ( R.ICost + R.NPages * RF / L.NTuples)";
      
      cost = (pn2->NumOfPages() * idx_selectivity) 
	     + (am->IndexPtr()->NumPages() * pn1->Cardinality())
	     + pn1->TotalCost();
  } 
  else if (am->IndexPtr()->GetSelectType() == selExact)
  {   
      // desc = " NTuples(Fetched) + (R.IHeight * L.NTuples) + L.Total ";
      desc = "L.Total + L.NTuples * ( R.ICost + NTuples(Fetched) / L.NTuples) ";
      
      cost = (pn2->Cardinality() * pn1->Cardinality()
	                         * idx_selectivity)
	     +  (am->IndexPtr()->NumPages() * pn1->Cardinality())
	     +  pn1->TotalCost();      
  }  
  else if (result->IndexOnly())
  {      
      // desc = " (R.INPages * RF) + (R.IHeight * L.NTuples) + L.Total ";
      desc = " L.Total + L.NTuples * (R.IHeight + R.INPages * RF/L.NTuples) "; 
      cost = (am->NumLeafPages()
	                * idx_selectivity)
	  + (am->IndexPtr()->NumPages() * pn1->Cardinality())
	      + pn1->TotalCost();      
  }  
  else if (am->IndexPtr()->IsClustered())
  {      
      // desc = " (R.INPages * RF) + (R.IHeight * L.NTuples) + (R.NPages * RF) + L.Total ";
      desc = " L.Total + L.NTuples * ( R.IHeight + R.INPages * RF/L.NTuples + R.NPages * RF / L.NTuples) "; 
      cost = (am->NumLeafPages()
	                * idx_selectivity)
              + (am->IndexPtr()->NumPages() * pn1->Cardinality())
	      + (pn2->NumOfPages() * idx_selectivity)
	      + pn1->TotalCost();      
  }  
  else 
  {      
      // desc = " (R.INPages * RF) + (L.NTuples * R.IHeight) + NTuples(Fetched) + L.Total ";
      desc = " L.Total + L.NTuples * ( R.IHeight + R.INPages * RF/L.NTuples +  NTuples(Fetched) / L.NTuples) ";
      cost =  (am->NumLeafPages()
	                * idx_selectivity)
              + (pn1->Cardinality() * am->IndexPtr()->NumPages())
              + (pn1->Cardinality() * pn2->Cardinality()
                        * idx_selectivity)
	      + pn1->TotalCost();
  }   

  // a little bit of a hack here.  the second number should probably be
  // the number of pages returned, but it might not matter.

  result->SetAccessMethod(Plan_AccessMethod("", 0, tupleOrder, attribute, NULL));
  result->GetAccessMethod().SetCost(cost); 

  result->SetDescription(desc);
  
  result->SetMemoryRequired(3);
 
  // add the result to the given solution list
  sol->Insert(result);
}

int IndexNestedLoops::CanBeApplied(PlanNode *pn1, PlanNode *pn2,
                               SelectList& applicableSelects,
						 SelectList& primaryPreds)
{
    if ( !pn1 || !pn2 || !pn2->RightChild()
     || applicableSelects.IsEmpty()){
          return FALSE;
     }

    pn2->children.GoHead();
    if ( pn2->children.CurrPtr()->IsPlanNode() )
     return FALSE;
    else
    {
     pn2->children.GoHead();

     Plan_AccessMethod *am = pn2->children.CurrPtr()->GetPlan_AccessMethod();

     if ( am &&  am->IndexPtr()){
          am->IndexPtr()->MostSelective(&applicableSelects, primaryPreds);
          if(!primaryPreds.IsEmpty()){
               return TRUE;
          }
     }   
    return FALSE;
    }
}








