//
// sortmerge.C
//

#include "sortmerge.h"
#include <math.h>
#include "opt_globals.h"


SortMerge::SortMerge():JoinMethod() 
{ 
    leftAttribute = rightAttribute = NULL;
    
    // activeJoinMethodList.Insert(this);
    name = "Sort Merge"; 
    costDescription = 
	"The cost estimate is computed using the following formula for \n"
"joining plan R and plan L, where R is the right subplan and L is the \n"
"left:\n\n"
"     L.TotalCost + R.NPages\n\n"
"If either or both of the subplans are not in an order such that they can \n"
"be merged, the cost of sorting is added in.  The cost of sorting a relation \n"
"is 4*NPages.  We assume, for the sake of the optimizer, that it will only take two \n"
"additional passes to sort the data.  This is not always realistic. ";
}


/* The cost function is defined assuming an equijoin is being
 * performed, therefore CanBeApplied returns true if the primary join
 * predicate is an equality predicate, and false otherwise.
 */
int SortMerge::CanBeApplied(PlanNode *pn1, PlanNode *pn2,
		   Select *primaryJoinPredicate)
{
    if (pn1 && pn2 && primaryJoinPredicate)
    {
	pn2->children.GoHead();
	
	if ( !pn2->children.CurrPtr()
	     || !pn2->children.CurrPtr()->GetPlan_AccessMethod() 
	     || (pn2->children.CurrPtr()->GetPlan_AccessMethod()->IndexPtr()
		 && ( pn2->children.CurrPtr()->GetPlan_AccessMethod()->GetSelectType() == selExact ) 
))
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

void SortMerge::MakePlanNode(PlanNode *pn1, PlanNode *pn2, 
			 const SelectList & sel,
			 Select *primaryJoinPredicate,
			 const AttributeList & al,
			 SolutionList *sol)
{
  PlanNode *result;
  OrderByList order = pn1->order;
  std::string desc;
  
  double cost = pn1->TotalCost();
  
  pn2->children.GoHead();  
  Plan_AccessMethod* am = pn2->children.CurrPtr()->GetPlan_AccessMethod();
  
  cost += am->CalculateCost(1);
  
  order.GoHead();
 
  if (order.IsEmpty() || (*order.CurrPtr()->attr) != (*leftAttribute)) // notsorted
  {
      desc += " 4L.NPages + ";      
      cost += pn1->NumOfPages() * 4;
  } 

  TupleOrder tupleOrder = am->Order();

  if (tupleOrder == Random 
      || rightAttribute != am->AttributePtr())
  {
      int np 
	  = pn2->GetAccessMethod().AttributePtr()->RelationPtr()->NumPages();
      
      desc += " 4R.NPages + ";      
      cost += np * 4;
  }

  desc += "L.TotalCost + ";

  if ( !am->IndexPtr() )
      desc += "R.NPages";
/*  else if ( am->IndexPtr()->GetSelectType() == selExact
	   && am->IndexPtr()->IsClustered())
      desc += "R.NPages + R.Height"; // should not be used
  else if ( am->IndexPtr()->GetSelectType() == selExact)
      desc += "R.NTuples + R.Height"; // should not be used */
  else if ( am->IndexPtr()->IsClustered() )
      desc += "R.IHeight + R.INPages + R.NPages"; 
  else
      desc += "R.IHeight + R.INPages + R.NTuples";
  
  result = NewPlanNode(pn1, pn2, sel, primaryJoinPredicate, al);

  tupleOrder = Ascending;
  Attribute *attribute = leftAttribute;

  result->SetAccessMethod(Plan_AccessMethod("", 0, tupleOrder, attribute, NULL));  
  result->GetAccessMethod().SetCost(cost);
  if (pn1->Cardinality() > pn2->Cardinality())
      result->SetMemoryRequired(1 + (int) sqrt( pn1->Cardinality()));
  else
      result->SetMemoryRequired(1 + (int) sqrt( pn2->Cardinality()));
  result->SetDescription(desc);
   
  sol->Insert(result);  
}









