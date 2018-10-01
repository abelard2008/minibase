//
// sol_tbl.h
//

/* This file contains the classes that maintain a hash table of lists.
 * The lists maintain partial and complete solutions for joining the
 * relations in a query.  The optimizer uses these classes for finding
  * query execution strategies.  The information stored using these
 * classes can also be referenced to provide the system user with
 * information about alternate query execution strategies.
 */

#ifndef	SOL_TBL_H
#define SOL_TBL_H

#include "planner.h"
#include "dmk.h"
#include <string>

/* The SolutionList class is a derived class from the List<PlanNode>
 * class.  This class maintains a list of PlanNodes with the ability of
 * automatically pruning off nodes that are not the cheapest for each
 * order of each attribute.  Pruning is done during insertion by either
 * automatically deleting a formerly cheapest solution for a particular
 * order, or not linking in the new node if it would not be the cheapest.
 * Pruning can be turned off and on.  With pruning off, any node that is
 * inserted is inserted at the head of the list regardless of cost or
 * order.  For pruning to work the Insert function needs to be used rather
 * than InsertHead, InsertTail, or InsertBeforeCurr.
 */
class SolutionList: public List<PlanNode>
{
private:
  int pruning;
//  Attribute *GetOrderedAttribute(const PlanNode & j);
public:
  SolutionList(): List <PlanNode>() { pruning = 1;}; /* turned pruning on */
  void PruningOff() { pruning = 0; };
  void PruningOn() { pruning = 1; };
  void TogglePruning() { pruning = !pruning; };
  int IsPruned() { return pruning; } ;
 
  PlanNode *Insert(PlanNode & pn);
  PlanNode *Insert(PlanNode *pn);

  void ShowList(bool markBest=FALSE);	// for display purposes
};



/* The SolutionTable class maintains a hash table of SolutionLists.  Each
 * SolutionList keeps a list of PlanNodes that represent joining the same
 * relations.  For example, if there are three relations A, B, and C to
 * be joined then the SolutionTable would look like this:
 *
 * Hash Value
 * (in binary)
 *   ABC
 *   ---   _
 *   000  |_| -> Empty
 *   001  |_| -> Solutions for accessing relation C
 *   010  |_| -> Solutions for accessing relation B
 *   011  |_| -> Solutions for joining B and C
 *   100  |_| -> Solutions for accessing relation A
 *   101  |_| -> Solutions for joining A and C
 *   110  |_| -> Solutions for joining A and B
 *   111  |_| -> Solutions for joining A, B, and C
 *
 * Each list of solutions is referenced by a list of relations.  The
 * function GetSolutionList(const List<Relation> &joinedRelations)
 * returns a pointer to the list that corresponds to the given list
 * of relations.  SolutionTable members are:
 *
 *      relations                array of pointers to Plan_Relations; it is used
 *                               for determining the hash value for a given
 *                               Plan_Relation or list of Plan_Relations
 *
 *      solutions                the hash table of SolutionLists
 *
 *      numOfRels                the number of relations in the relations
 *                               array
 *
 *      GetHashValue             returns the hash value for a given Plan_Relation
 *
 *      GetSolutionList(const List<Plan_Relation> &joinedPlan_Relations)
 *                               returns a pointer to the SolutionList that
 *                               corresponds to the Plan_Relations in the given
 *                               list of Plan_Relations
 *
 *      GetSolutionList(Plan_Relation * r)
 *                               returns a pointer to the SolutionList that
 *                               corresponds to the given Plan_Relation
 */
class SolutionTable
{
private:
  Array <Plan_Relation *> relations;
  Array <SolutionList> solutions;
  Array <SolutionList> access;
  
  int numOfRels;

  uint GetHashValue(Plan_Relation * r);
  int NumberOfRels(uint nHashValue);

public:
  SolutionTable() {  } ;
  SolutionTable(State *initialState);

  SolutionList *GetSolutionList(const List<Plan_Relation> &joinedPlan_Relations);
  SolutionList *GetSolutionList(Plan_Relation * r);
  SolutionList *GetAccessList(Plan_Relation * r);
  

  PlanNode *FindNode(int nID);

  void Reset();
  
  void PruningOn(int level);
  void PruningOff(int level);
  int IsPruned(int level);

  void ShowTable();	// for display purposes
};

#endif





