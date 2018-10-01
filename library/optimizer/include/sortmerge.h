//
// sortmerge.h
//

#ifndef _SORTMERGE_H
#define _SORTMERGE_H

#include "joinmeth.h"

/* The following is the join method class for a sort merge join.  The
 * cost function is defined assuming an equijoin is being performed,
 * therefore CanBeApplied returns true if the primary join predicate
 * is an equality predicate, and false otherwise.
 * 
 * The cost function used is described in the constructor definition
 * in sortmerge.C.  Look at the initialization of the costDescription
 * member variable.  The resulting tuple order is determined by the
 * sorted order of the input relations.  If neither input relation is
 * sorted then two new PlanNodes are created, one with ascending and
 * the other descending order on the join attributes.
 */
class SortMerge: public JoinMethod
{
  private:
    Attribute* leftAttribute;
    Attribute* rightAttribute;
  public:
    SortMerge();
    
    int CanBeApplied(PlanNode *pn1, PlanNode *pn2,
		     Select *primaryJoinPredicate);
    void MakePlanNode(PlanNode *pn1, PlanNode *pn2, 
		      const SelectList & sel,
		      Select *primaryJoinPredicate,
		      const AttributeList & al,
		      SolutionList *sol);
    
    //    JoinFunction *GetJoinFun()  { return (JoinFunction *) 0; }
};

/* The following is the join method class for a sort merge join with
 * the resulting relation realized to disk.  The CanBeApplied function
 * is borrowed from the basic sort merge, since this join method can
 * be applied in only the same situations.
 * 
 * The cost function used is described in the constructor definition
 * in sortmerge.C.  Look at the initialization of the costDescription
 * member variable.  The resulting tuple order is determined by the
 * sorted order of the input relations.  If neither input relation is
 * sorted then two new PlanNodes are created, one with ascending and
 * the other descending order on the join attributes.

class SortMergeRealized: public SortMerge
{
public:
  SortMergeRealized();

  void MakePlanNode(PlanNode *pn1, PlanNode *pn2, 
                    const SelectList & sel,
		    Select *primaryJoinPredicate,
		    const AttributeList & al,
		    SolutionList *sol);

//    JoinFunction *GetJoinFun()  { return (JoinFunction *) 0; }
};
 */

#endif





