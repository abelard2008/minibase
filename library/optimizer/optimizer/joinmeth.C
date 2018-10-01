//
// joinmeth.c
//

#include "joinmeth.h"
#include "da_types.h"


JoinMethodListType masterJoinMethodList;
JoinMethodListType activeJoinMethodList;

JoinMethod::JoinMethod()
{
  // masterJoinMethodList.Insert(this);
  costDescription = "No cost function description given";
}

// See planner.h for declaration of PlanNode constructor. 
PlanNode *JoinMethod::NewPlanNode(PlanNode *pn1, PlanNode *pn2, 
				  const SelectList & sl,
				  Select *primaryJoinPredicate,
				  const AttributeList & al)
{
  PlanNode *result = new PlanNode(this, pn1, pn2, sl, primaryJoinPredicate, al);
  if (!result) FatalError(MEM);

  return result;
}


void ShowJoinMethods(int withCost)
{
  int i;
  for (masterJoinMethodList.GoHead(), i = 1; 
       masterJoinMethodList.CurrPtr();
       masterJoinMethodList.GoNext(), i++) {
    std::cout << i << " " << masterJoinMethodList.CurrPtr()->Name();
    if (activeJoinMethodList.Goto(masterJoinMethodList.CurrPtr()))
      std::cout << ".:\n";
    else
      std::cout << " (Inactivated).:\n";
    if (withCost) {
      std::cout << masterJoinMethodList.CurrPtr()->CostDescription() << ".:\n";
    }
  }
  std::cout << "end - join methods.:\n";
}

void ToggleJoinMethod(int num)
{
  for (masterJoinMethodList.GoHead(); 
       masterJoinMethodList.CurrPtr() && num > 1;
       masterJoinMethodList.GoNext(), num--) {}
  JoinMethod *curr = masterJoinMethodList.CurrPtr();
  if (curr) {
    if (activeJoinMethodList.Goto(curr)) 
      activeJoinMethodList.Detach();
    else
      activeJoinMethodList.Insert(curr);
  }
}


template class ListNode<JoinMethod>;
template class DList<JoinMethod>;
template class ListBase<JoinMethod>;



