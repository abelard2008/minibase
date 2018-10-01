//
// optimizer.c
//

#include "optimizer.h"
#include "joinmeth.h"
#include "opt_globals.h"
#include <math.h>
#include <assert.h>
#include "nestloop.h"


Optimizer::Optimizer(State *initialState): solutionTable(initialState)
{
  state = initialState;
}


void Optimizer::Reset()
{
  solutionTable.Reset();
  ResetNextNodeID();
  for (state->joins.GoHead(); state->joins.CurrPtr(); state->joins.GoNext())
    ExpandPlan_Relation(state->joins.CurrPtr());

}


Optimizer::~Optimizer()
{ 
  if (state) { 
      state->selects.Clean(); 
      state->project.Clean();
      state->neededAttributes.Clean();
      state->groupby.Clean();
      state->joins.Clean();
      state->order.Clean();
      state->omits.Clean();    
    delete state; 
  } 
}
	          

void Optimizer::ExpandPlan_Relation(Plan_Relation * r)
{
  SolutionList *solL = solutionTable.GetSolutionList(r);
  SolutionList *accessL = solutionTable.GetAccessList(r);
  
  for (r->accessMethodList.GoHead(); r->accessMethodList.CurrPtr();
       r->accessMethodList.GoNext()) {
      // goto that particular accessmethod
      if (!state->omits.Goto(r->accessMethodList.CurrPtr())) {
	
	AttributeList al(&state->neededAttributes);
	SelectList sl(&state->selects);

	MakeSelectAndProjectLists(sl, al, NULL, NULL, r);

	double indexSelectivity = 1;
	// double cost;
	
	Index* ourIndex = r->accessMethodList.CurrPtr()->IndexPtr();
	SelectList out_list(TRUE);     // most selective set
	
	if (  ourIndex )
	{	    
          
          // We have to find most selective set of predicates.
          // It is a set, not a single predicate, because we may
          // have multiple key indecies.

          double thisSelectivity = ourIndex->MostSelective(&sl, out_list);
          if ( thisSelectivity < indexSelectivity){
               indexSelectivity = thisSelectivity;
               assert(out_list.Size() >= 1);
          }
	} 
	else if ( r->accessMethodList.CurrPtr()->Order() != Random)
	{
	    Select* cs;
	    Select* mostSelective = NULL;
	    Attribute* at = r->accessMethodList.CurrPtr()->AttributePtr();
	    
	    for ( sl.GoHead(); (cs=sl.CurrPtr()); sl.GoNext())
		if ( cs->Contains(at) 
		    && cs->GetExpr()->IsElement()
		    && (cs->GetExpr()->GetElem()->GetType() != selUndefined)
		    && (cs->GetSelectivity() < indexSelectivity))
		{		    
		    mostSelective = cs;
		    indexSelectivity = cs->GetSelectivity();
		}
		if(mostSelective){
			out_list.InsertHead(mostSelective);
		}
	}

	PlanNode pn(r->accessMethodList.CurrPtr(), sl, al, out_list);
	pn.SetAccessMethod(Plan_AccessMethod(r->accessMethodList.Curr()));
		
	pn.GetAccessMethod().SetCost(r->accessMethodList.CurrPtr()->CalculateCost(indexSelectivity,pn.IndexOnly()));
	
	pn.SetDescription(r->accessMethodList.Curr().GetDescription(pn.IndexOnly()));
	
	// std::cerr << "calculated cost for: " << pn.GetAccessMethod().Name()
	//    " " << pn.GetAccessMethod().Cost() << endl;
	
	//	if ( !ourIndex || mostSelective || ourIndex->GetSelectType() != selExact )

	solL->Insert(pn);

	// now, we'll add it to the list of plain access plans.  This
        // list will be used by join methods to find right-most access
        // methods (this is because the regular "basic" access method
        // list is inappropriate for our purposes.
 
	List<Select> empty;
	
	PlanNode pn2(r->accessMethodList.CurrPtr(), empty, r->attributeList, NULL, true);
	pn2.SetAccessMethod(Plan_AccessMethod(r->accessMethodList.Curr()));
	pn2.SetDescription(r->accessMethodList.Curr().GetDescription());
	pn2.GetAccessMethod().SetCost(0); // cost will be in the next level.
	
	accessL->InsertHead(pn2);
	
    }
  }
}


// temporary, for debugging
void ShowPlan_Relations(Plan_RelationList *rl)
{
  Plan_RelationList temp(rl);

  for (temp.GoHead(); temp.CurrPtr(); temp.GoNext())
    std::cout << temp.Curr().Name() << "   ";
 // std::cout << "\n";
}


void Optimizer::MakeSelectAndProjectLists(SelectList & sl, AttributeList & al,
				  PlanNode *pn1, PlanNode *pn2, Plan_Relation *r)
{
    SelectList tempSelList1(TRUE), tempSelList2(&sl);
    if (pn1)
	pn1->GetSelectList(tempSelList1);
    if (pn2)
	pn2->GetSelectList(tempSelList1);
    
    tempSelList2 - &tempSelList1;           // Delete the selects from
    // tempSelList2 that have been 
    // applied already in pn1 and
    // pn2 and their
    // descendents

  sl.UnLink();

  Plan_RelationList rl(TRUE);
  if (r)
    rl.Insert(r);
  if (pn1)
    pn1->GetPlan_RelationList(rl);
  if (pn2)   
    pn2->GetPlan_RelationList(rl);

  // Look at all of the select predicates in tempSelList2.  Then insert into 
  // sl the ones that can be applied to the relations in rl.
  for (tempSelList2.GoHead(); tempSelList2.CurrPtr(); tempSelList2.GoNext())
    if (tempSelList2.CurrPtr()->CanDoSelect(rl))
      sl.InsertTail(tempSelList2.CurrPtr());

  AttributeList neededAttributes(&al), availableAttributes(TRUE);
  if (r)
    availableAttributes + &r->attributeList;
  if (pn1)
    availableAttributes + &pn1->project;
  if (pn2)
    availableAttributes + &pn2->project;

  tempSelList2 - &sl;

  for (tempSelList2.GoHead(); tempSelList2.CurrPtr(); tempSelList2.GoNext())
    tempSelList2.CurrPtr()->GetAttributeList(neededAttributes);

  al.UnLink();
  for (availableAttributes.GoHead(); availableAttributes.CurrPtr();
       availableAttributes.GoNext())
      if (neededAttributes.Goto(availableAttributes.CurrPtr()))
	  al.InsertTail(availableAttributes.CurrPtr()); 
}


void Optimizer::ROptimize(State currentState)
{
  Plan_Relation *r;
  
  SolutionList *relationSL;
  
  SolutionList *partialSL, *fullSL;

  

  fullSL = solutionTable.GetSolutionList(currentState.joins);
  if (fullSL->IsEmpty())
      for (currentState.joins.GoHead(); currentState.joins.CurrPtr(); currentState.joins.GoNext())
      {
	  r = currentState.joins.Detach();
	  
	  ROptimize(currentState);
	  relationSL = solutionTable.GetAccessList(r);
	  partialSL =
	      solutionTable.GetSolutionList(currentState.joins);
	  
	  for (partialSL->GoHead(); partialSL->CurrPtr();
	       partialSL->GoNext())
	      for (relationSL->GoHead(); relationSL->CurrPtr();
		   relationSL->GoNext())
	      {
		  SelectList sl(&currentState.selects);
		  AttributeList al(&currentState.project);
		  MakeSelectAndProjectLists(sl, al, partialSL->CurrPtr(),
					    relationSL->CurrPtr());

		  // has a join been successfully done?
		  
		  JoinMethod* aJoin;

		  for (activeJoinMethodList.GoHead();
		       (aJoin = activeJoinMethodList.CurrPtr());
		       activeJoinMethodList.GoNext())
		  {
		      // std::cout << aJoin->Name() << endl;
		      
		      int doneOnce = FALSE;
		      
		      // check -- can we join with any of the 
		      // plausable selections as a join condition?
               
			 if(aJoin->Name() ==
                     "Index Nested Loops with Right SubTree as Inner"){
               
                     // Special processing is needed for indexed access.
                     // Index can be an multikey index and therefore
                     // there may be more than one primary predicate.

                      IndexNestedLoops* thisJoin = (IndexNestedLoops*) aJoin;
                      SelectList primaryPreds(TRUE);

                      // primaryPreds is a list of primary join predicates.

                      if (thisJoin->CanBeApplied(partialSL->CurrPtr(),
                                     relationSL->CurrPtr(), sl, primaryPreds)){

                          thisJoin->MakePlanNode(partialSL->CurrPtr(),
                                     relationSL->CurrPtr(),
                                   sl, primaryPreds,
                                     al, fullSL);
                          doneOnce = TRUE;
                      }
                }
                else{

                // There is only one primary predicate

		      for (sl.GoHead(); sl.CurrPtr(); sl.GoNext())
		      {
			  if (aJoin->CanBeApplied(partialSL->CurrPtr(),
						  relationSL->CurrPtr(),
						  sl.CurrPtr())) 
			  {			      
			      aJoin->MakePlanNode(partialSL->CurrPtr(), 
						  relationSL->CurrPtr(),
						  sl, sl.CurrPtr(),
						  al, fullSL);
			      doneOnce = TRUE;
			  }  
		      }
		      // no possible primary condition - so check if we
		      // can do one with none.	      		      
		      if (!doneOnce 
			  && aJoin->CanBeApplied(partialSL->CurrPtr(),
						 relationSL->CurrPtr(),
						 NULL))
			  aJoin->MakePlanNode(partialSL->CurrPtr(), 
					      relationSL->CurrPtr(),
					      sl, NULL,
					      al, fullSL);
                }
		  }		  
	      }	  
	  currentState.joins.InsertBeforeCurr(r);
      }
}

PlanNode *Optimizer::GetBestSolution()
{
  PlanNode *cheapest = NULL, *current;
  SolutionList *sl = solutionTable.GetSolutionList(state->joins);
  sl->GoHead();
  cheapest = sl->CurrPtr();
  for ( ;(current = sl->CurrPtr()); sl->GoNext())
  {
      if (current->TotalCost() < cheapest->TotalCost())    
	  cheapest = current;
  }
  return cheapest;
}

void Optimizer::Aggregation()
{
    if ( !state->containsAggregates) // needed == projection list
	return;

    PlanNode *current;
    SolutionList *sl = solutionTable.GetSolutionList(state->joins); 
    // current solution list
    
    for ( sl->GoHead(); (current = sl->CurrPtr()); sl->GoNext())
    {
	current->having = state->having;	
	current->groupby = state->groupby;
	if (current->groupby.IsEmpty() )
	    current->SetCardinality( 1);
	else
	    current->SetCardinality( current->Cardinality() / 3 );
	
	// completely arbitrary.  however, it really isn't important in
        // our situation.
	
	current->SetDoAgg(true);
	current->project = state->project; // use attributes project list
    }
}

// Are any of them sorted?  In what order?
void Optimizer::CheckOrdering() 
{
    bool sortme = (!state->order.IsEmpty() || state->distinct
		   || state->aggDistinct || !state->groupby.IsEmpty() );

    if (!sortme)
	return;

    PlanNode *current;
    SolutionList *sl = solutionTable.GetSolutionList(state->joins); 
    // current solution list

    for ( sl->GoHead(); (current = sl->CurrPtr()); sl->GoNext())
    {
	OrderByList& access = current->order;
	OrderByList& need = state->order;	  
	double scost = 0;
		    
	if ( current->order.IsEmpty() ) // not sorted
	{
	    ulong numPages = current->NumOfPages();
	    scost = numPages * 4; 
	    current->SetSortCost( scost, true, state->distinct);
	    
	    current->order.Clean();
	    
	    for ( need.GoHead(); need.CurrPtr(); need.GoNext())
	    {
		OrderAttr* oa = new OrderAttr;
		oa->attr = need.CurrPtr()->attr;
		oa->tupleOrder = need.CurrPtr()->tupleOrder;		
		current->order.InsertTail(oa);
	    }		  
	}
	else // already sorted, but is it any good?
	{	

	    for(access.GoHead(), need.GoHead();
		access.CurrPtr()
		&& need.CurrPtr()
		&& *(access.CurrPtr()) == *(need.CurrPtr());
		access.GoNext(), need.GoNext());
	    
	    if ( need.CurrPtr() ) // nope.  Not really sorted  
	    {
		ulong numPages = current->NumOfPages();
		scost = numPages * 4; 
		current->SetSortCost( scost, true, state->distinct);
		access.Clean();
	    
		for ( need.GoHead(); need.CurrPtr(); need.GoNext())
		{
		    OrderAttr* oa = new OrderAttr;
		    (*oa) = (*need.CurrPtr());
		    access.InsertTail(oa);
		}		
	    }    
	}
	
	// check -- if we didn't sort, is it already sorted on all of our
	// columns?  (then we have to add the cost-to-sort)
	if ( (state->distinct || state->aggDistinct) && scost == 0)
	{
	    bool found = true;
	    
	    for (current->project.GoHead(); 
		 current->project.CurrPtr() && found;
		 current->project.GoNext() )
	    {
		found = false;
		for (access.GoHead();
		     access.CurrPtr() && !found;
		     access.GoNext())
		{
		    if (access.CurrPtr()->attr)
			found = 
			    (*access.CurrPtr()->attr == *current->project.CurrPtr());
		}
	    }

	    if (!found) // we will have to sort.
	    {
		ulong numPages = current->NumOfPages();
		scost = numPages * (4);
	    current->SetSortCost( scost, true, state->distinct);
	    }
	    
	    
	}	
     if(!state->groupby.IsEmpty() && scost == 0){

          // Check if we have to sort due to group by requirement.
          // Group by list should be in the same order as the projection list.
          // Consequently, if state->order is not the same as projection list
          // order we have to sort.

          AttributeList& need(state->groupby);
          need.GoHead();
          access.GoHead();
          while(need.CurrPtr() && access.CurrPtr()){
               if(*access.CurrPtr()->attr != *need.CurrPtr()){
                    break;
               }
               need.GoNext();
               access.GoNext();
          }
          if(need.CurrPtr()){

               // We have to sort

               ulong numPages = current->NumOfPages();
			scost = numPages * (4);
			current->SetSortCost(scost, true, state->distinct);
          }
     }
    }    
}


PlanNode *Optimizer::GetPlanByID(int nID)
{
  return solutionTable.FindNode(nID);
}


SolutionList *Optimizer::GetSolutionList()
{
  return solutionTable.GetSolutionList(state->joins);
}



void Optimizer::ShowSolutions()
{
  SolutionList *sl;
  solutionTable.ShowTable();
  sl = GetSolutionList();
  for (sl->GoHead(); sl->CurrPtr(); sl->GoNext()) {
    std::cout << sl->CurrPtr()->GetAccessMethod().NodeID() << "\n";
  }
  std::cout << "end - root list.:\n";
}


void Optimizer::ToggleOmits(int nodeID)
{
  int found = FALSE;
  for (state->omits.GoHead(); state->omits.CurrPtr() && !found; ) {
    found = (state->omits.CurrPtr()->NodeID() == nodeID);
    if (!found)
      state->omits.GoNext();
  }

  if (found)
    state->omits.Detach();
  else {
    Plan_Relation *pr;
    for (state->joins.GoHead(); 
	 (pr = state->joins.CurrPtr()) && !found; ) {
      for (pr->accessMethodList.GoHead();
	   pr->accessMethodList.CurrPtr() && !found; ) {
	found = (pr->accessMethodList.CurrPtr()->NodeID() == nodeID);
	if (!found)
	  pr->accessMethodList.GoNext();
      }
      if (!found)
	state->joins.GoNext();
    }
    if (found)
      state->omits.InsertTail(pr->accessMethodList.CurrPtr());
  }
}

void Optimizer::Optimize() 
{ 
    Optimizer::Reset();
    ROptimize(*state);
    CheckOrdering();
    Aggregation();
}

void Optimizer::ShowTable()
{
    solutionTable.ShowTable();
}   // for debugging purposes

void Optimizer::PruningOn(int level)
{
    solutionTable.PruningOn(level);
}

void Optimizer::PruningOff(int level)
{
    solutionTable.PruningOff(level);
}

void Optimizer::IsPruned(int level)
{
    solutionTable.IsPruned(level);
}

void Optimizer::TogglePruning(int level)
{ 
    if (solutionTable.IsPruned(level)) 
	solutionTable.PruningOff(level);
    else solutionTable.PruningOn(level);
}










