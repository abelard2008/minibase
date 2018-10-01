//
// merge.c
//

#include "joinmeth.h"


class Merge: public JoinMethod
{
public:
  Merge():JoinMethod() { name = "Merge"; } ;

  int CanBeApplied(PlanNode *pn1, PlanNode *pn2,
		   Select *primaryJoinPredicate);
  void MakePlanNode(PlanNode *pn1, PlanNode *pn2, 
                    const SelectList & sel,
		    Select *primaryJoinPredicate,
		    const AttributeList & al,
		    SolutionList *sol);

//    JoinFunction *GetJoinFun()  { return (JoinFunction *) 0; }
};



int Merge::CanBeApplied(PlanNode *pn1, PlanNode *pn2,
		   Select *primaryJoinPredicate)
{
  if (pn1 && pn2 && primaryJoinPredicate) 
    if (pn1->GetAccessMethod().Order() != Random &&
	pn1->GetAccessMethod().Order() == pn2->GetAccessMethod().Order()) {

      SelectExpr *expr;
      SelectElem *elem;

      if ( (expr = primaryJoinPredicate->GetExpr()) )
        if (expr->IsElement() && (elem = expr->GetElem()))
	  if (elem->GetType() == selExact && elem->Arg1() && elem->Arg2())
	    if (!elem->Arg1()->IsLiteral() && !elem->Arg2()->IsLiteral())
	      if ((pn2->GetAccessMethod().AttributePtr() == elem->Arg1()->AttributePtr() && 
		   pn1->GetAccessMethod().AttributePtr() == elem->Arg2()->AttributePtr()) ||
		  (pn2->GetAccessMethod().AttributePtr() == elem->Arg2()->AttributePtr() &&
		   pn1->GetAccessMethod().AttributePtr() == elem->Arg1()->AttributePtr()))
		return TRUE;
    }

  return FALSE;
}


void Merge::MakePlanNode(PlanNode *pn1, PlanNode *pn2, 
			 const SelectList & sel,
			 Select *primaryJoinPredicate,
			 const AttributeList & al,
			 SolutionList *sol)
{
  PlanNode *result = NewPlanNode(pn1, pn2, sel, primaryJoinPredicate, al);
  TupleOrder tupleOrder = pn1->GetAccessMethod().Order();
  Attribute *attribute = pn1->GetAccessMethod().AttributePtr();
  double cost = pn1->GetAccessMethod().Cost() + pn1->Cost() +
                              pn2->GetAccessMethod().Cost() + pn2->Cost();

  result->SetAccessMethod(Plan_AccessMethod("", 0, tupleOrder, attribute, NULL));  
  result->GetAccessMethod().SetCost(cost);
  
  sol->Insert(result);
}


// Merge merge_dummy;

