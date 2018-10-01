//
// planner.h
//

/* This file contains the classes that together with the classes in
 * query.h are used to represent a query execution plan.  A query
 * execution plan is structured as a tree that looks like:
 *
 *                 J
 *               /  \        
 *             J     J        Where J's are instances of the PlanNode
 *           /  \    |        class, and AM's are instances of the 
 *         J     J   AM       Plan_AccessMethod Class (declared in query.h).
 *         |     |            Links in the tree are acheived using the
 *        AM     AM           Child and ChildList classes.
 *
 *
 * Although the current implementation of the optimizer produces only
 * plans that can be represented by binary left deep trees, the ChildList
 * class is structured such that bushy trees and/or arbitray arity can
 * be represented using the same data structures.  e.g.:
 *
 *               J                                J
 *            /    \                           /  |  \
 *          J       J                       J     J   AM
 *        /  \     /  \                   / | \   | \  
 *      AM   AM   AM   AM               AM  AM AM AM AM
 *
 *         Bushy tree                       Multiple arity
 *
 * note: Current optimizer does not produce plans that look like this.
 *
 * Classes and Types declared in this file:
 *
 *      Child                       stores a pointer to either an instance
 *                                  of Plan_AccessMethod or an instance of
 *                                  PlanNode
 *
 *      ChildList                   maintains a list of instances of Child
 *
 *      PlanNode                    stores a description of a join
 */

#ifndef _PLANNER_H
#define _PLANNER_H

#include "query.h"

// #define _STANDALONE_
//additional includes

#ifndef _STANDALONE_
#include "iterator.h"
#include "new_error.h"
#endif

// forward references
class PlanNode;
class JoinMethod;


/* The Child class stores a pointer that points to either an instance of
 * PlanNode or an instance of Plan_AccessMethod.  The Child class exists because
 * the query execution strategy that the optimizer produces is a tree
 * stucture with internal nodes of type PlanNode and leaf nodes of type
 * Plan_AccessMethod.  If the pointer points to a PlanNode then isPlanNode is 
 * true, otherwise isPlanNode is false.
 */
class Child
{

public:
  bool IsPlanNode();

  // Assignment methods
  Plan_AccessMethod *operator =(Plan_AccessMethod *am);
  PlanNode *operator =(PlanNode *pn);

  Plan_AccessMethod *GetPlan_AccessMethod();
  PlanNode *GetPlanNode();

  int operator==(const Child & c);

private:
  bool isPlanNode;
  union {
    PlanNode *pnPtr;
    Plan_AccessMethod *amPtr;
  };

};


/* The ChildList class maintains a list of Child nodes.  In addition to
 * constructors and a destructor, methods are provided for inserting
 * a pointer to an Plan_AccessMethod and for inserting a pointer to a PlanNode.
 */
class ChildList: public List<Child>
{
public:
  Child *InsertPlan_AccessMethod(Plan_AccessMethod *am);
  Child *InsertPlanNode(PlanNode *pn);

  ChildList() : List<Child>() { }; 

  // Non-destructive copy constructor
  ChildList(const ChildList & cl) : List<Child>(cl) {  }; 

  ~ChildList();
};


/* The PlanNode class stores information that describes how a Join is
 * to be performed.  It provides the name of the join method to be used,
 * the join predicate (stored as a list of select predicates), a list of
 * children to join (children are either an access method for a relation
 * or another PlanNode), a list of attributes to project on after
 * joining.  Estimates for the cost of performing the join and the
 * cardinality of the result are present only because the optimizer
 * makes use of those feilds.  The PlanNode class consists of the 
 * following members:
 *
 *      joinMethod                   a pointer to the JoinMethod applied
 *                                   to make the node
 *
 *      predicate                    a list of select/join predicates to
 *                                   be applied
 *
 *      primaryJoinPredicate         a pointer to an element in the predicate
 *                                   list, this is to remove ambiguity for
 *                                   the query executor
 *
 *      project                      a list of attributes to project on
 *  
 *      children                     stores pointers to the nodes children
 *
 *      accessMethod                 access method available for accessing
 *                                   the resulting realtion
 *
 *      cost                         estimated cost of building the relation,
 *                                   usually 0 unless realized to disk
 *
 *      realize                      TRUE if the relation is to be realized on
 *                                   disk, FALSE otherwise
 *
 *      cardinality                  estimated cardinality of resulting 
 *                                   relation
 *
 *      PlanNode(Plan_AccessMethod *am, const SelectList & sl, 
 *               const AttributeList & al)
 *                                   constructor for node with single 
 *                                   Plan_AccessMethod for a child
 *                                   Arguments:
 *                                     am  pointer to child
 *                                     sl  list of predicates to be applied
 *                                     al  list of attributes to project on
 *
 *      PlanNode(JoinMethod *iJoinMethod, PlanNode *pn1, PlanNode *pn2, 
 *               const SelectList & sl, Select *iPrimaryJoinPredicate, 
 *               const AttributeList & al)
 *                                   constructor for a node with PlanNodes as
 *                                   children
 *                                   Arguments:
 *                                     iJoinMethod   pointer to a JoinMethod
 *                                                   which is the join method
 *                                                   to use
 *                                     pn1, pn2      pointers to the children
 *                                     sl            list of predicates to be 
 *                                                   applied 
 *                                     iPrimaryJoinPredicate
 *                                                   pointer to the primary
 *                                                   join predicate
 *                                     al            attributes to project on
 *
 *      PlanNode(const PlanNode & pn)
 *                                   copy constructor
 *
 *      GetSelectList                returns a list of select predicates from
 *                                   the PlanNode tree that this is the root of
 *
 *      GetPlan_RelationList         Returns a list of relations in rl from the 
 *                                   PlanNode tree that this is the root of
 *
 *      The remaining functions are mainly for access to protected member 
 *      variables and are more or less self explanatory
 */
class PlanNode
{
protected:
  JoinMethod *joinMethod; // the join method to use

  Select *primaryJoinPredicate;
  SelectList primaryPredicates; // Multikey index has more than 1 primary

  Plan_AccessMethod accessMethod;  // The access method available for accessing
                                   // the resulting relation
 
  double cost;            // estimated cost of building relation 
  double sortCost;        // estimated cost of sorting relation
  
  int realize;             // true if resulting relation is realized on disk
  
  ulong cardinality;      // estimated
  ulong preCardinality; // the cardinality before any selects
   
  int memoryRequired;     // amount of memory required for this element
                          // currently unused.
  bool doSort, doDistinct; // do I need to sort or make unique?
  bool doAgg; // do we need to calculate aggregates? 

  std::string costDescription;
  bool rightChild; // a "fake" plan node, to hook a join method
                   // to an accessMethod.=
  bool indexOnly; // true if the plan is index-only
  
   
public: 
  OrderByList order; // sorted order of plan
  SelectList predicate;   // the join/select predicate

  AttributeList project;  // attributes to project on
  AttributeList groupby; // if appropriate to group.
  SelectList having; // (same with having)
  
  ChildList children;     // list of descendents

  bool IndexOnly() { return indexOnly;};
    
  bool RightChild() { return rightChild; } ;
  // is this plan node meant to be a right child to 
  // a join method?

  PlanNode(Plan_AccessMethod *am,
	   const SelectList & sl,
	   const AttributeList & al,
	   Select *iPrimaryJoinPredicate = NULL,
	   bool isRight = false);

  PlanNode(Plan_AccessMethod *am,
	   const SelectList & sl,
	   const AttributeList & al,
	   SelectList& PrimaryPredicates);

  PlanNode(JoinMethod *iJoinMethod,
	   PlanNode *pn1,
	   PlanNode *pn2,
	   const SelectList & sl, 
	   Select *iPrimaryJoinPredicate,
	   const AttributeList & al);

  PlanNode(JoinMethod *iJoinMethod,
	   PlanNode *pn1,
	   PlanNode *pn2,
	   const SelectList & sl, 
	   SelectList& primaryJoinPredicates,
	   const AttributeList & al);

  PlanNode(const PlanNode & pn); // copy constructor

  virtual ~PlanNode() {}; // this should delete order list-members
  
#ifndef _STANDALONE_
  // additonal functions
  Iterator* interpret(AttributeList& attrs, Status& status);
void LeafInfo(
              std::string& amName, std::string& relName, std::string& indName, AttributeList& al,int& fldNum,
      int& index_only);
  
  void ShowRecursive();
#endif
  
  int MemoryRequired() { return memoryRequired;  };		 
  bool DoAgg() { return doAgg; };
  bool SetDoAgg(bool count) { return doAgg = count;};
        
  JoinMethod *GetJoinMethod() { return joinMethod; };
  Select *PrimaryJoinPredicate() { return primaryJoinPredicate; };
  Plan_AccessMethod & GetAccessMethod() { return accessMethod; };
  double Cost() { return cost; };
  void SetCost(double nCost) { cost = nCost; realize = TRUE;};
  void SetSortCost(double nCost, bool nSort, bool nDistinct);

  void SetMemoryRequired(int mr) { memoryRequired = mr;}  ;      
  void SetDescription(std::string descript) { costDescription = descript;};
  
  double SortCost() { return sortCost; };

  double TotalCost() { return sortCost + accessMethod.Cost() + cost;};
  int Realize() { return realize; };
  ulong SetCardinality(ulong card) { return (cardinality = card);};
        
  ulong Cardinality() { return cardinality; };
  
  int GetTupleSize();
  ulong NumOfPages();
  
  void SetAccessMethod(const Plan_AccessMethod & iAccessMethod); 
  
  void GetSelectList(SelectList & sl); // Returns a list of select predicates 
                                         // in sl from the PlanNode tree that 
				       // this is the root of

  void GetPlan_RelationList(Plan_RelationList & rl);  
                                            // Returns a list of relations in
                                            // rl from the PlanNode tree that
                                            // this is the root of

  int operator==(const PlanNode & /*pn*/)
    { return FALSE; };   // the == operator is defined because the List<PlanNode>
                         // class requires it, however it is not used
			   
  void Show(bool isBest=FALSE);

  void ShowTree();
};

#endif







