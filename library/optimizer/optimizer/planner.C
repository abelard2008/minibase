//
// planner.c
//

// #include "nestloop.h"

#include "planner.h"
#include "joinmeth.h"
// #include "display.h"
#include <stdio.h>
#include "StringUtils.h"
#include "opt_globals.h"
#include <math.h>
#include <assert.h>
/*
 * child member function
 *
 */

Plan_AccessMethod *Child::operator =(Plan_AccessMethod *am)
{
  isPlanNode = FALSE;
  return amPtr = am;
}

PlanNode *Child::operator =(PlanNode *pn)
{
  isPlanNode = TRUE;
  return pnPtr = pn;
}

Plan_AccessMethod *Child::GetPlan_AccessMethod()
{
    if ( !isPlanNode)
	return amPtr;
    else
	return NULL;
}

PlanNode *Child::GetPlanNode()
{
    if (isPlanNode)
	return pnPtr;
    else
	return NULL;
}

bool Child::IsPlanNode()
{
    return isPlanNode;
}


int Child::operator ==(const Child & c)
{
  if (isPlanNode == c.isPlanNode) {
    if (isPlanNode)
      return (pnPtr == c.pnPtr);
    else
      return (amPtr == c.amPtr);
  }
  else 
    return FALSE; 
}

template class List<Child>;
template class ListNode<Child>;
template class DList<Child>;
template class ListBase<Child>;

Child *ChildList::InsertPlan_AccessMethod(Plan_AccessMethod *am)
{
  Child c;

  c = am;
  return Insert(c);
};


Child *ChildList::InsertPlanNode(PlanNode *pn)
{
  Child c;

  c = pn;
  return Insert(c);
}

ChildList::~ChildList()
{
    Clean();
}

PlanNode::PlanNode(Plan_AccessMethod *am, const SelectList & sl, 
	const AttributeList & al, SelectList& PrimaryPredicates){ 
	PrimaryPredicates.GoHead();
	PlanNode* tmp = new PlanNode(am, sl, al, PrimaryPredicates.CurrPtr()); 
	memcpy(this, tmp, sizeof(PlanNode));
	primaryPredicates = &PrimaryPredicates;
}

PlanNode::PlanNode(Plan_AccessMethod *am, const SelectList & sl, const AttributeList & al, Select *iPrimaryJoinPredicate, bool isRight) :
					 primaryPredicates(TRUE),
					 accessMethod(*am),
					 order(TRUE),
					 predicate(TRUE),
					 project(TRUE),
					 groupby(TRUE),
					 having(TRUE),
                          children()
{
  double selectivity;
  doAgg = false;
  joinMethod = NULL;
  primaryJoinPredicate = iPrimaryJoinPredicate;

  project = &al;
  predicate = &sl;  
  Attribute* a= am->AttributePtr();
  Index* ind = am->IndexPtr();

  // if we are considering index only plans, see if this plan is a potential
  // index only one.
  // a plan is index only if the only attributes that remain are those that
  // are on the index, and all of the predicates can be applied by only
  // looking at the index.
  if (ind && Optimizer_IndexOnly)
  {      
	 std::string relName = a->RelationPtr()->Name();
      indexOnly = true;
      AttributeList al(ind->Attributes());
      Attribute* currAtt;
      Attribute* currProj;

      project.GoHead();
      while(indexOnly && (currProj = project.CurrPtr())){

               if(relName == currProj->RelationPtr()->Name()){
                    indexOnly = false;
                    al.GoHead();
                    while((currAtt = al.CurrPtr())){
                         if(currAtt->Name() == currProj->Name()){
                              indexOnly = true;
                              break;
                         }
                         al.GoNext();
                    }
               }
               project.GoNext();
      }

	 predicate.GoHead();
	 while(indexOnly && predicate.CurrPtr()){

               AttributeList sA; // select's attributes
               sA = predicate.CurrPtr()->GetAttributeList();
               for(sA.GoHead(); indexOnly && sA.CurrPtr(); sA.GoNext()){

                    Attribute* currSel = sA.CurrPtr();
                    if (currSel->RelationPtr()->Name() == relName){
                         indexOnly = false;
                         al.GoHead();
                         while((currAtt = al.CurrPtr())){
                              if(currAtt->Name() == currSel->Name()){
                                   indexOnly = true;
                                   break;
                              }
                              al.GoNext();
                         }
                    }
               }
               predicate.GoNext();
          }
  }
  else indexOnly = false;
  
  cost = sortCost = 0;
  rightChild = isRight;
  
  realize = FALSE;
  doSort = doDistinct = false;
  memoryRequired = 1;
  costDescription = "";

     if((ind = am->IndexPtr())){

          // This might be a multikey index and the order of the attributes
          // is the order in which they are present in index.

          AttributeList al(ind->Attributes());
          al.GoHead();
          Attribute* currAtt;
          OrderAttr* oa;
          order.Clean();
          while((currAtt = al.CurrPtr())){
               oa = new OrderAttr;
               oa->attr = new Attribute(*currAtt);
               oa->tupleOrder = am->Order();
               order.InsertTail(oa);
               al.GoNext();
          }
     }
  else if (am->Order() != Random) // not a random access method!
  {
      OrderAttr* oa = new OrderAttr;
      oa->attr = am->AttributePtr();
      oa->tupleOrder = am->Order();
      order.Clean();     
      order.InsertTail(oa);
  }
  
  for (selectivity = 1, predicate.GoHead(); predicate.CurrPtr(); 
       predicate.GoNext()) 
    selectivity = selectivity * predicate.CurrPtr()->GetSelectivity();

  preCardinality = am->AttributePtr()->RelationPtr()->Cardinality();
  
  cardinality = (ulong) (selectivity * preCardinality);

  if(cardinality < 1) cardinality = 1; // always at least one tuple.

  memoryRequired = 2; // only two pages should be necessary
  


  children.InsertPlan_AccessMethod(am);
}


PlanNode::PlanNode(JoinMethod *iJoinMethod, PlanNode *pn1, PlanNode *pn2, 
		   const SelectList & sl, SelectList& primaryJoinPredicates, 
		   const AttributeList & al){
     primaryJoinPredicates.GoHead();
     PlanNode* tmp = new PlanNode(
         iJoinMethod, pn1, pn2, sl, primaryJoinPredicates.CurrPtr(), al); 
     memcpy(this, tmp, sizeof(PlanNode));
     primaryPredicates = &primaryJoinPredicates;
}

PlanNode::PlanNode(JoinMethod *iJoinMethod, PlanNode *pn1, PlanNode *pn2, 
		   const SelectList & sl, Select *iPrimaryJoinPredicate, 
		   const AttributeList & al): 
		   primaryPredicates(TRUE),
		   accessMethod(),
		   order(TRUE),	
		   predicate(TRUE),   
		   project(TRUE),
		   groupby(TRUE),
		   having(TRUE),
		   children()
{
     primaryPredicates.InsertHead(iPrimaryJoinPredicate);

    rightChild = false;
    
    double selectivity;
    
    joinMethod = iJoinMethod;
    primaryJoinPredicate = iPrimaryJoinPredicate;
    
    cost = sortCost =  0;
    memoryRequired = 1;
    costDescription = "";
    pn2->children.GoHead();
    
    Attribute* a= pn2->children.CurrPtr()->GetPlan_AccessMethod()->AttributePtr();

    project = &al;    
    predicate = &sl;

    if (a && Optimizer_IndexOnly) 
    {      
	indexOnly = true;
	for ( project.GoHead(); indexOnly && project.CurrPtr(); project.GoNext())
	{
	    if ( (a->Name() != project.CurrPtr()->Name())
		&& ( a->RelationPtr()->Name() == (project.CurrPtr()->RelationPtr()->Name())))
		indexOnly = false;
	}
     for ( predicate.GoHead(); indexOnly && predicate.CurrPtr(); predicate.GoNext())
      {
	  AttributeList sA; // select's attributes
	  sA = predicate.CurrPtr()->GetAttributeList();
	  for ( sA.GoHead(); indexOnly && sA.CurrPtr(); sA.GoNext())
	  {
	      if ( sA.CurrPtr()->Name() != a->Name()
		  && sA.CurrPtr()->RelationPtr()->Name() == a->RelationPtr()->Name())
		  indexOnly = false;
	  }
      }

    }
    else indexOnly = false;
    
    
    realize = FALSE;
    doSort = doDistinct = false;
  

    
    // set default cardinality as if a cross product was applied
    preCardinality = pn1->Cardinality() * pn2->Cardinality();

//    if (pn1->Cardinality() > pn2->Cardinality()) 
//	memoryRequired = 1 + (int) sqrt( pn1->Cardinality());
//   else
//	memoryRequired = 1 + (int) sqrt( pn2->Cardinality());
  		
    for (selectivity = 1, predicate.GoHead(); predicate.CurrPtr(); 
	 predicate.GoNext())
	selectivity = selectivity * predicate.CurrPtr()->GetSelectivity();

    cardinality = (ulong)(selectivity * preCardinality);

    if(cardinality < 1) cardinality = 1; // always at least one tuple.
    

     doAgg = false;
    children.InsertPlanNode(pn1);
    children.InsertPlanNode(pn2);
}
   
PlanNode::PlanNode(const PlanNode & pn): 
   primaryPredicates(&pn.primaryPredicates),
   accessMethod(pn.accessMethod),
   costDescription(pn.costDescription),
   order(&pn.order),
   predicate(&pn.predicate),
   project(&pn.project),
   groupby(&pn.groupby),
   having(&pn.having),
   children(pn.children)
{
  joinMethod = pn.joinMethod;
  primaryJoinPredicate = pn.primaryJoinPredicate;
  cost = pn.cost;
  sortCost = pn.sortCost;
  indexOnly = pn.indexOnly;
  
  realize = pn.realize;
  cardinality = pn.cardinality;
  preCardinality = pn.preCardinality;
  memoryRequired = pn.memoryRequired;
  doSort = pn.doSort;
  doDistinct = pn.doDistinct;  
  rightChild = pn.rightChild; 
  doAgg = false;
  
} 


void PlanNode::SetAccessMethod(const Plan_AccessMethod & iAccessMethod)
{ 
    accessMethod = iAccessMethod; 
    if (!project.Goto(accessMethod.AttributePtr())) {
	double cost = accessMethod.Cost();
	accessMethod = DefaultAM;
	accessMethod.SetCost(cost);
    }
    //	project.Insert(accessMethod.AttributePtr());

    Index* ind;
     if((ind = accessMethod.IndexPtr())){
          AttributeList al(ind->Attributes());
          al.GoHead();
          Attribute* currAtt;
          OrderAttr* oa;
          order.Clean();
          while((currAtt = al.CurrPtr())){
               oa = new OrderAttr;
               oa->attr = new Attribute(*currAtt);
               oa->tupleOrder = accessMethod.Order();
               order.InsertTail(oa);
               al.GoNext();
          }
     }
  else if (accessMethod.Order() != Random) // not a random access method!
  {
      OrderAttr* oa = new OrderAttr;
      oa->attr = accessMethod.AttributePtr();
      oa->tupleOrder = accessMethod.Order();
      order.Clean();     
      order.InsertTail(oa);          
  }
}


int PlanNode::GetTupleSize()
{
   int result = 0;
  for (project.GoHead(); project.CurrPtr(); project.GoNext())
    result += project.CurrPtr()->Size();
  return result; 
}


void PlanNode::GetSelectList(SelectList & sl)
{
  sl + &predicate;
  for (children.GoHead(); children.CurrPtr(); children.GoNext())
    if (children.CurrPtr()->IsPlanNode())
      children.CurrPtr()->GetPlanNode()->GetSelectList(sl);
}


void PlanNode::GetPlan_RelationList(Plan_RelationList & rl)
{
  for (children.GoHead(); children.CurrPtr(); children.GoNext())
    if (children.CurrPtr()->IsPlanNode())
      children.CurrPtr()->GetPlanNode()->GetPlan_RelationList(rl);
    else
      rl.InsertTail(children.CurrPtr()->GetPlan_AccessMethod()->AttributePtr()->RelationPtr());
}



void PlanNode::Show(bool isBest)
{
  std::cout << accessMethod.NodeID() << '\n';;

  if (isBest)
      std::cout << "* Best Plan *" << std::endl;

  if (joinMethod) 
  {
      std::cout << "  " << joinMethod->Name() + "\n";
  }
  
  std::cout << "  Total Cost = Cost + SortCost = "
       << TotalCost();
  std::cout << "\n  NTuples = NTuples(Input) * RF = " << cardinality << "\n"; 

  if ( doSort )
      std::cout << "  Sort the result." << std::endl;

  if ( doDistinct)
      std::cout << "  Make Distinct." << std::endl;

  if (indexOnly)
      std::cout << "  Index Only Scan" << std::endl;


  std::cout << "[";
  std::cout << "\nFrom Catalog/Child Node:" << std::endl;
 
  if (primaryJoinPredicate)
      std::cout << "  NTuples(Fetched) = ";
  else
      std::cout << "  NTuples(Fetched) = ";
  
  std::cout << preCardinality << "\n";

  if ( primaryJoinPredicate)
      std::cout << "  ReductionFactor(Primary) = RF(Primary) = "
	   << primaryJoinPredicate->GetSelectivity() << std::endl;
  else
      std::cout << "  ReductionFactor(Primary) = RF(Primary) = 1.0 " << std::endl;
  
  std::cout << std::endl<< "Calculated in this node:" << std::endl;

  double selectivity;
  
  std::cout << " Reduction Factor = RF = ";
  
  int count = 0;
  
  for (selectivity = 1, predicate.GoHead(); predicate.CurrPtr(); 
       predicate.GoNext())
  {     
      selectivity = selectivity * predicate.CurrPtr()->GetSelectivity();
      if ( count != 0 ) std::cout << " * ";
      std::cout << "RF" << count;
      count++;
  }
      
  if (count != 0)
      std::cout << " = ";
  else
      std::cout << " RF0 = ";
     
  std::cout << selectivity << std::endl;

  if (primaryJoinPredicate)
  {
      ulong indexCardinality;
      
      indexCardinality = 
	  long(preCardinality * primaryJoinPredicate->GetSelectivity());
      
      if (indexCardinality < 1)
	  std::cout << "  NTuples(Fetched) = NTuples * RF(Primary) = " << 1 << "\n";
      else
	  std::cout << "  NTuples(Fetched) = NTuples * RF(Primary) = "
	      << indexCardinality << "\n";      
  } 
  else
  {
      std::cout << "  NTuples(Fetched) = NTuples * RF(Primary) = " 
           << preCardinality << std::endl;
  }

  std::cout << "  NTuples(result) = NTuples(Input) * RF ";
  
  if ( ! groupby.IsEmpty() )
      std::cout << " / 3 " ;      
  std::cout << "= " << cardinality << "\n"; 
  
  std::cout << "  Result Tuple Size = TSize = " << GetTupleSize() << "\n";
  std::cout << "  Page Size = PSize = " << pageSize << std::endl;
  
  std::cout << "  Result Number of Pages = Result.NPages = Result.NTuples * (TSize / PSize) = " 
       << NumOfPages() << std::endl;
  
  if (realize) {
      std::cout << "  Cost to Build  = " << cost << "\n";
      std::cout << "  Cost to Access = " << costDescription << accessMethod.Cost();
  } else
      std::cout << "  Cost  = " << costDescription << " = " << accessMethod.Cost();
  
      std::cout << "\n  SortCost = ";
      
      if ( doSort || doDistinct)
	  std::cout << "4 Result.NPages = ";
      
      std::cout   << SortCost();
      
  std::cout << "\n  Buffer Pool Pages Required =  " << memoryRequired ; 

  std::cout << "\n  Realized: ";
  if (realize) std::cout << "True\n";  
  else std::cout << "False";
  std::cout << "\n]";  

  std::cout << "\nPredicate:";
  if (predicate.IsEmpty())
    std::cout << "\n>   all (S0 = 1.0)";
  else
  {
      bool primaryExists = false;
      int sub = 0;
     
      for (predicate.GoHead(); predicate.CurrPtr(); ) {
	  std::cout << "\n>   " << predicate.CurrPtr()->MakeString();
	  if (predicate.CurrPtr() == primaryJoinPredicate)
	  {
	      primaryExists = true;
	  }
	  std::cout << " (RF" << sub << " = " 
	       << predicate.CurrPtr()->GetSelectivity() << " )";
	  sub++;
	  
	  predicate.GoNext();
	  if (predicate.CurrPtr())
	      std::cout << ",";
      }
      if (!primaryExists){
	  std::cout << "\n (No Primary Predicate)";
      }
	 else{
	  std::cout << "\n Primary Predicates:\n";
	  primaryPredicates.GoHead();
	  Select* curr_pred = primaryPredicates.CurrPtr();
	  std::cout << ">   ";
	  while(curr_pred){
	   std::cout << curr_pred->MakeString();
	   primaryPredicates.GoNext();
	   curr_pred = primaryPredicates.CurrPtr();
	   if(curr_pred){
		   std::cout << ", ";
	   }
	  }
	 }
   }
  
  std::cout << "\n\nProject on:";
  int cnt = 0;
  for (project.GoHead(); project.CurrPtr(); project.GoNext()) {
      if ( cnt % 3 == 0 ) std::cout << "\n>   ";
      else std::cout << " , ";
      cnt++;
      
      std::cout << project.CurrPtr()->FullName();
    }
  
  if ( ! order.IsEmpty() )
  {      
      std::cout << "\n\nOrder by:";
  }
  
  for ( order.GoHead(); order.CurrPtr(); order.GoNext())
  {
      if ( order.CurrPtr()->attr)
      {
	  std::cout << "\n>  " << order.CurrPtr()->attr->FullName();
	  switch (order.CurrPtr()->tupleOrder)
	  {
	    case Ascending:
	      std::cout << "  (Ascending) ";
	      break;
	    case Descending:
	      std::cout << "  (Descending) ";
	      break;
	    default:
	      break;
	  }
      }
  }
  
  if ( ! groupby.IsEmpty() )
  {
      std::cout << "\nGroup by:";
  }

  cnt = 0;
  for ( groupby.GoHead(); groupby.CurrPtr(); groupby.GoNext())
  {
      if ( cnt % 3 == 0 ) std::cout << "\n>   ";
      else std::cout << " , ";
      cnt++;
      
      std::cout << groupby.CurrPtr()->FullName();
  }

  if (! having.IsEmpty() )
  {
      std::cout << "\nHaving:";

      for (having.GoHead(); having.CurrPtr(); )
      {
	  std::cout << "\n>   " << having.CurrPtr()->MakeString();

	  having.GoNext();
	  if (having.CurrPtr())
	      std::cout << ",";
      }
  }
    
  Child *childPtr;
  std::cout << "\nChildren:";
  for (children.GoHead(); (childPtr = children.CurrPtr()); children.GoNext())
    if (childPtr->IsPlanNode())
    {
	if ( childPtr->GetPlanNode()->RightChild())
	{
	    childPtr->GetPlanNode()->children.GoHead();
	    
	    Child *grandchild = childPtr->GetPlanNode()->children.CurrPtr();
	    if (grandchild->IsPlanNode())
	    {
		// bad scene
	    }
	    else
		std::cout << ' ' << grandchild->GetPlan_AccessMethod()->NodeID();
	}
	else
	    std::cout << ' ' << childPtr->GetPlanNode()->accessMethod.NodeID();
    }  
    else
      std::cout << ' ' << childPtr->GetPlan_AccessMethod()->NodeID();
  std::cout << '\n';

  std::cout << ".:\n";
}

  
void PlanNode::ShowTree()
{
  std::cout << accessMethod.NodeID();
  Child *childPtr;
  for (children.GoHead(); (childPtr = children.CurrPtr()); children.GoNext())
    if (childPtr->IsPlanNode())
      std::cout << ' ' << childPtr->GetPlanNode()->accessMethod.NodeID();
    else
      std::cout << ' ' << childPtr->GetPlan_AccessMethod()->NodeID();
  std::cout << '\n';
  for (children.GoHead(); (childPtr = children.CurrPtr()); children.GoNext())
    if (childPtr->IsPlanNode())
      childPtr->GetPlanNode()->ShowTree();
}

ulong PlanNode::NumOfPages()
{
    return (ulong(ceil (cardinality * (GetTupleSize() / double(pageSize)))));
}

void PlanNode::SetSortCost(double nCost, bool nSort, bool nDistinct)
{
    sortCost = nCost; 
    doSort = nSort;
    doDistinct = nDistinct;
}







