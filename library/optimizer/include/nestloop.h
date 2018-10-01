#ifndef _NESTLOOP_H_
#define _NESTLOOP_H_

#include "joinmeth.h"
#include "opt_globals.h"

/* The following is the join method class for a nested loops join with
 * the right subplan as the inner relation.  The right subplan
 * corresponds to pn2 in calls to member functions CanBeApplied and
 * MakePlanNode.  
 *
 * This version of nested loops _does not_ support indicies on the right
 * plan, and
 * therefore CanBeApplied checks for a filescan access method
 *
 * The cost function used is described in the constructor definition
 * in nestloop.C.  Look at the initialization of the costDescription
 * member variable.  The resulting tuple order is the same tuple
 * ordering that the outer relation has.
 */
class NestedLoopsRightSubtreeInner: public JoinMethod
{
private:
//    static JoinFunction JF;

public:
    NestedLoopsRightSubtreeInner();

    int CanBeApplied(PlanNode *pn1, PlanNode *pn2,
		     Select *primaryJoinPredicate);
    void MakePlanNode(PlanNode *pn1, PlanNode *pn2, 
		      const SelectList & sel, Select *primaryJoinPredicate,
		      const AttributeList & al,
		      SolutionList *sol);
    
//    JoinFunction *GetJoinFun() { return &JF; }; 
};

/*
 * page-oriented nested loops.
 *
 * Currently 7/24/95 doesn't support indicies
 *
 */
class PageNestedLoops: public JoinMethod
{
private:
//    static JoinFunction JF;

public:
    PageNestedLoops();

    int CanBeApplied(PlanNode *pn1, PlanNode *pn2,
		     Select *primaryJoinPredicate);
    void MakePlanNode(PlanNode *pn1, PlanNode *pn2, 
		      const SelectList & sel, Select *primaryJoinPredicate,
		      const AttributeList & al,
		      SolutionList *sol);
    
//    JoinFunction *GetJoinFun() { return &JF; }; 
};


// IndexNestedLoops
// 7/31/95

class IndexNestedLoops: public JoinMethod
{
private:
//    static JoinFunction JF;
//    double costforone;

public:
    IndexNestedLoops();
//    double CostForOne() { return costforone; };

    int CanBeApplied(PlanNode *pn1, PlanNode *pn2,
               SelectList& applicableSelects,
               SelectList& primaryPreds);

    void MakePlanNode(PlanNode *pn1, PlanNode *pn2, 
		      const SelectList & sel, SelectList& primaryPreds,
		      const AttributeList & al,
		      SolutionList *sol);
    
//    JoinFunction *GetJoinFun() { return &JF; }; 
};

#endif





