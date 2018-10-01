//
// hashjoin.C
//

#include "hashjoin.h"
#include <math.h>
#include "opt_globals.h"
//#include <string>

HashJoin::HashJoin() :JoinMethod() 
{ 
    leftAttribute = rightAttribute = NULL;
    
    // activeJoinMethodList.Insert(this);
    name = "Hash Join"; 
    costDescription = 
	"The cost estimate is computed using the following formula for \n"
"joining plan R and plan L, where R is the right subplan and L is the \n"
"left:\n\n"
"     L.TotalCost + 2(L.NPages) + 3(R.NPages) \n\n"
"Note that we assume that each partition will fit into memory on the second \n"
"phase.  Also, note that the first read through of L is include as part of \n"
"the TotalCost.";
}


/* The cost function is defined assuming an equijoin is being
 * performed, therefore CanBeApplied returns true if the primary join
 * predicate is an equality predicate, and false otherwise.
 */
int HashJoin::CanBeApplied(PlanNode *pn1, PlanNode *pn2,
		   Select *primaryJoinPredicate)
{
    if (pn1 && pn2 && primaryJoinPredicate)
    {
	pn2->children.GoHead();
	
	if ( !pn2->children.CurrPtr()
	     || !pn2->children.CurrPtr()->GetPlan_AccessMethod() 
	     || pn2->children.CurrPtr()->GetPlan_AccessMethod()->IndexPtr()
//		 && pn2->children.CurrPtr()->GetPlan_AccessMethod()->GetSelectType()
//	            == selExact 
	   )
	    return FALSE;
	
	SelectExpr *expr;
	SelectElem *elem = 0;
	
	if ( (expr = primaryJoinPredicate->GetExpr()) )
	    if (expr->IsElement() && (elem = expr->GetElem()))
		if ( (elem->GetType() == selExact)
		    && (!elem->Arg1()->IsLiteral() && !elem->Arg2()->IsLiteral()))
		{
		    Attribute* a1 = elem->Arg1()->AttributePtr();
		    Attribute* a2 = elem->Arg2()->AttributePtr();
		    
		    // is each one from a different slot?
		    if (pn1->project.Goto(*a1) && pn2->project.Goto(*a2))
		    {
			leftAttribute = a1;
			rightAttribute = a2;		    
			return TRUE;
		    }		
		    else if (pn1->project.Goto(a2)
			     && pn2->project.Goto(a2))
		    {
			leftAttribute = a2;
			rightAttribute = a1;
			return TRUE;
		    }		
		    else
			return FALSE;
		}	
    }
    return FALSE;
}

void HashJoin::MakePlanNode(PlanNode *pn1, PlanNode *pn2, 
			 const SelectList & sel,
			 Select *primaryJoinPredicate,
			 const AttributeList & al,
			 SolutionList *sol)
{
  PlanNode *result;
  std::string desc;
  
  double cost = pn1->TotalCost();
  
  pn2->children.GoHead();  
  Plan_AccessMethod* am = pn2->children.CurrPtr()->GetPlan_AccessMethod();
  
  cost += am->CalculateCost(1);
  
  desc += " 2L.NPages + 2R.NPages ";
 
  cost += pn1->NumOfPages() * 2;
 
  int np = pn2->GetAccessMethod().AttributePtr()->RelationPtr()->NumPages();
      
  cost += np * 2;
  
  desc += "+ L.TotalCost + ";

  if ( !am->IndexPtr() )
      desc += "R.NPages";
/*
// all the rest of these are never used.  just junk
  else if ( am->IndexPtr()->GetSelectType() == selExact
	   && am->IndexPtr()->IsClustered())
      desc = "R.NP + R.IC";
  else if ( am->IndexPtr()->GetSelectType() == selExact)
      desc = "R.C + R.IC";
  else if ( am->IndexPtr()->IsClustered() )
      desc = "R.IC + R.ILP + R.NP";
  else
      desc = "R.IC + R.ILP + R.C"; */
  
  result = NewPlanNode(pn1, pn2, sel, primaryJoinPredicate, al);

  Attribute *attribute = leftAttribute;
  
  result->SetAccessMethod(Plan_AccessMethod("", 0, Random, attribute, NULL));  
  result->GetAccessMethod().SetCost(cost);
  if (pn1->Cardinality() < pn2->Cardinality())
      result->SetMemoryRequired(1 + (int) sqrt( pn1->Cardinality()));
  else
      result->SetMemoryRequired(1 + (int) sqrt( pn2->Cardinality()));
  result->SetDescription(desc);
   
  sol->Insert(result);  
}



