//
// hashjoin.h
//

#ifndef _hashjoin_H
#define _hashjoin_H

#include "joinmeth.h"

/* The following is the join method class for a hash merge join.  The
 * cost function is defined assuming an equijoin is being performed,
 * therefore CanBeApplied returns true if the primary join predicate
 * is an equality predicate, and false otherwise.
 * 
 * The cost function used is described in the constructor definition
 * in hashmerge.C.  Look at the initialization of the costDescription
 * member variable.  The resulting tuple order is always random
 * (IE, any previous sort order is lost)
 */
class HashJoin: public JoinMethod
{
  private:
    Attribute* leftAttribute;
    Attribute* rightAttribute;
  public:
    HashJoin();
    
    int CanBeApplied(PlanNode *pn1, PlanNode *pn2,
		     Select *primaryJoinPredicate);
    void MakePlanNode(PlanNode *pn1, PlanNode *pn2, 
		      const SelectList & sel,
		      Select *primaryJoinPredicate,
		      const AttributeList & al,
		      SolutionList *sol);
    
    //    JoinFunction *GetJoinFun()  { return (JoinFunction *) 0; }
};

#endif





