//
// sol_tbl.c
//

#include "sol_tbl.h"
#include <iostream>



PlanNode *SolutionList::Insert(PlanNode & pn)
{
  Plan_AccessMethod *am1 = 0, *am2;
  int deleteCurr = FALSE;

  if (pruning) {
    am1 = &pn.GetAccessMethod();
    if (!pn.project.Goto(am1->AttributePtr())) 
      am1->SetOrder(Random);
  }
  if (!pruning || IsEmpty()) return InsertHead(pn);
  else {
    if (am1->Order() == Random)
      for (GoHead(); CurrPtr(); ) {  // for each node in this list...

        am2 = &CurrPtr()->GetAccessMethod();
	if (am2->Order() == Random) {
	  if ((am1->Cost() <= am2->Cost()) &&
	      (pn.Cost() <= CurrPtr()->Cost())) 
	    deleteCurr = TRUE;
	  else if (am1->Cost() >= am2->Cost() && pn.Cost() >= CurrPtr()->Cost())
	    return NULL; }
	else if (am1->Cost() >= am2->Cost() && pn.Cost() >= CurrPtr()->Cost())
	  return NULL;
	
        if (deleteCurr) { 
	  Delete(); 
	  deleteCurr = FALSE;
	} else 
	  GoNext();
      }
    else
      for (GoHead(); CurrPtr(); ) {  // for each node in this list...

	am2 = &CurrPtr()->GetAccessMethod();
        if (am2->Order() == Random && (am1->Cost() <= am2->Cost() &&
					  pn.Cost() <= CurrPtr()->Cost()))
	    deleteCurr = TRUE;
	else if (am1->AttributePtr() == am2->AttributePtr() && 
		 am1->Order() == am2->Order())
	  if (am1->Cost() <= am2->Cost() && pn.Cost() <= CurrPtr()->Cost())
	    deleteCurr = TRUE;
	  else if (am1->Cost() >= am2->Cost() &&
		   pn.Cost() >= CurrPtr()->Cost())
	    return NULL;
	if (deleteCurr) { 
	  Delete(); 
	  deleteCurr = FALSE;
	} else 
	  GoNext();
      }
    return InsertHead(pn);
  }
}


PlanNode *SolutionList::Insert(PlanNode *pn)
{
  PlanNode *result;
  
  result = Insert(*pn);
  delete pn;
  return result;
}

void SolutionList::ShowList(bool markBest)
{
    PlanNode* best=NULL;
    
    if(markBest)
    {	
	for ( GoHead() ; CurrPtr(); GoNext())
	    if ( !best || CurrPtr()->TotalCost() < best->TotalCost() )
		best = CurrPtr();
    }
    
    for (GoHead(); CurrPtr(); GoNext()) {	
	CurrPtr()->Show(CurrPtr() == best);
    }  
}


SolutionTable::SolutionTable(State *initialState):
		relations(initialState->joins.Size()),
		solutions(1 << initialState->joins.Size()),
                access(initialState->joins.Size())
{
  for (numOfRels = 0, initialState->joins.GoHead(); numOfRels < (int)relations.Size();
	numOfRels++, initialState->joins.GoNext())
    relations[numOfRels] = initialState->joins.CurrPtr();
}


PlanNode *SolutionTable::FindNode(int nID)
{
  for (int i = (1 << numOfRels) - 1; i > 0; i--)
    for (solutions[i].GoHead(); solutions[i].CurrPtr(); solutions[i].GoNext())
      if (solutions[i].CurrPtr()->GetAccessMethod().NodeID() == nID)
	return solutions[i].CurrPtr();
  return NULL;
}


void SolutionTable::Reset()
{
    // clean the solutions list
    for (int i = 1; i < (1 << numOfRels); i++)
      solutions[i].Clean();
    //clean the access list
    for (int i = 0; i < numOfRels; i++)
	access[i].Clean();  
}


void SolutionTable::PruningOn(int level) 
{ 
  if (level > 0 && level < (1 << numOfRels))
    solutions[level].PruningOn(); 
}


void SolutionTable::PruningOff(int level) 
{ 
  if (level > 0 && level < (1 << numOfRels))
    solutions[level].PruningOff(); 
}


int SolutionTable::IsPruned(int level) 
{   
  if (level > 0 && level < (1 << numOfRels))
    return solutions[level].IsPruned(); 
  return 0;  
} 

uint SolutionTable::GetHashValue(Plan_Relation * r)
{
  int relNum;

  for (relNum = 0; relNum < numOfRels && relations[relNum] != r; relNum++);
  return (1 << relNum);
};

SolutionList *SolutionTable::GetSolutionList(const List<Plan_Relation> &joinedPlan_Relations)
{
  uint result = 0;
  List<Plan_Relation> temp(&joinedPlan_Relations);

  for (temp.GoHead(); temp.CurrPtr(); temp.GoNext())
    result += GetHashValue(temp.CurrPtr());

  //std::cout << "Hash value: " << result << "\n";

  return &solutions[result];
};


SolutionList *SolutionTable::GetSolutionList(Plan_Relation * r)
{
    return &solutions[GetHashValue(r)];
}

SolutionList *SolutionTable::GetAccessList(Plan_Relation * r)
{
    int relNum;
    
    for ( relNum = 0; relNum < numOfRels && relations[relNum] != r; relNum++);
    
    return &access[relNum];
}

int SolutionTable::NumberOfRels(uint nHashValue)
{
  int nResult = 0;
  while (nHashValue > 0) {
    if (nHashValue & 1)
      nResult++;
    nHashValue = nHashValue >> 1;
  }
  return nResult;
}


void SolutionTable::ShowTable()
{
  for (uint relNum = 0; relNum < (uint)numOfRels; relNum++) {
    std::cout << "start - relation access method list.:\n";
    std::cout << relations[relNum]->Name() << "\n";
    for (relations[relNum]->accessMethodList.GoHead();
	 relations[relNum]->accessMethodList.CurrPtr();
	 relations[relNum]->accessMethodList.GoNext()) {
      relations[relNum]->accessMethodList.CurrPtr()->Show();
    }
  }
  std::cout << "end - access methods.:\n";

  for (int n = 1; n <= numOfRels; n++) { 
    std::cout << "start - level " << n << "\n";
    for (uint i = 1; i < (uint)(1 << numOfRels) ; i++) 
      if (NumberOfRels(i) == n) {
	std::cout << "start - row " << i << "\n";
	for (int relNum = 0, comma = FALSE; relNum < numOfRels; relNum++)
	  if (i & (1 << relNum)) {
	    if (comma) std::cout << ",";
	    std::cout << relations[relNum]->Name();
	    comma = TRUE;
	  }
	std::cout << "\n";
	std::cout << "pruning: ";
	if (solutions[i].IsPruned())
	  std::cout << "on\n";
	else
	  std::cout << "off\n";
	solutions[i].ShowList(n == numOfRels);
      }
  }
  std::cout << "end - table.:\n";
}

template class List<PlanNode>;
template class ListNode<PlanNode>;
template class DList<PlanNode>;
template class ListBase<PlanNode>;

template class Array<Plan_Relation *>;
template class Array<SolutionList>;












