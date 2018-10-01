//
// optimizer.h
//

#ifndef _OPTIMIZER_H_
#define _OPTIMIZER_H_



#include "sol_tbl.h"
//#include "display.h"




/* The Optimizer class is the main class of the optimizer as a whole.
 * An instance of Optimizer is constructed with a pointer to an instance of
 * the State class (declared in query.h) which represents the query to be
 * optimized.  The Optimizer constructor sets the data member called state
 * to point to the initial query to be optimized.  The constructor also
 * initializes the data member solutionTable, which maintains partial and
 * complete execution strategies (see sol_tbl.h).  A call to the member
 * function Optimize() performs the query optimization.  After calling
 * Optimize() a call to GetBestSolution() returns a pointer to the root
 * node of the execution strategy predicted to be the cheapest.  The
 * Optimizer class consists of the following members:
 *
 *      solutionTable                stores partial an complete execution
 *                                   strategies
 *
 *      treeList                     stores a list of XShowTree(s) being
 *                                   displayed by XShowSolutions
 *
 *      state                        points to the query to be optimized
 *
 *      ExpandPlan_Relation          used internally to make initial
 *                                   entries in the solutionTable for
 *                                   single relations
 *
 *      MakeSelectAndProjectLists    determines which selections and 
 *                                   projections can be applied given
 *                                   two PlanNodes or a Plan_Relation
 *
 *      ROptimize                    recursive optimization procedure,
 *                                   called by Optimize
 *
 *      Optimizer()                  default constructor, required by at
 *                                   least some compilers, but should not
 *                                   be called
 *
 *      Optimizer(State *initialState)  constructor invoked with a pointer
 *                                   to a represention the query to be
 *                                   optimized
 *
 *      Reset                        resets the optimizer object
 *
 *      Optimize                     computes query execution plans
 *
 *      GetBestSolution              returns a pointer to the query
 *                                   execution plan estimated to be
 *                                   the cheapest to execute 
 *
 *      GetPlanByID                  returns a pointer to the query
 *                                   execution plan that has the 
 *                                   given id
 *
 *      GetSolutionList              returns a pointer to the list of
 *                                   current solutions 
 *
 *      PruningOn                    turns pruning on for a given level
 *                                   in the solutionTable if it is not
 *                                   already on
 *
 *      PruningOff                   turns pruning off for a given level
 *                                   in the solutionTable if it is not
 *                                   already off
 *
 *      IsPruned                     determines if a given level in the
 *                                   solutionTable is currently being pruned
 *
 *      TogglePruning                switches pruning between off and on
 *                                   for a given level in the solutionTable
 *
 *      ToggleOmits                  adds or removes access methods to
 *                                   be considered during optimization
 *
 *      ShowSolutions                displays the solutions in a format
 *                                   understandable by minibase.tcl
 *                                   
 */

/* The solutionTable member maintains a hash table of SolutionLists.
 * Each SolutionList keeps a list of PlanNodes that represent joining
 * the same relations.  For example, if there are three relations A,
 * B, and C to be joined then the SolutionTable would look like this:
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
 * The optimizer builds up this table using a Selinger style
 * algorithm.  The table is constructed in a bottom up fashion.  In
 * the above example, the lists numbered 001, 010, and 100 are filled
 * first.  Then all of the possible methods for joining B and C are
 * considered and added to the list numbered 011.  PlanNodes that are
 * estimated to be more costly plans than others in the same list,
 * that also have the same order, are pruned upon insertion to the
 * list.  This continues until the list corresponding to joining all
 * of the relations is filled.  Selections and projections are done as
 * early as possible (i.e. - at the lowest possible level in the above
 * table).
 */

class Optimizer
{
private:
  SolutionTable solutionTable;
  State *state;
  void ExpandPlan_Relation(Plan_Relation * r);
  void MakeSelectAndProjectLists(SelectList & sl, AttributeList & al,
				 PlanNode *pn1 = NULL, PlanNode *pn2 = NULL, 
				 Plan_Relation *r = NULL);
  void ROptimize(State currentState);
public:
  Optimizer() { state = NULL; };
  Optimizer(State *initialState);
  ~Optimizer();

  void Reset();
  void Optimize();
  PlanNode *GetBestSolution();
  PlanNode *GetPlanByID(int nID);
  SolutionList *GetSolutionList();

  void ShowTable() ;  // for debugging purposes

  void PruningOn(int level);
  void PruningOff(int level); 
  void IsPruned(int level);
  void TogglePruning(int level) ;

  void ToggleOmits(int nodeID);

  void ShowSolutions();
  void CheckOrdering(); 
  void Aggregation();
};

#endif
