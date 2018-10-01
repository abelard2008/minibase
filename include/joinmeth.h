//
// joinmeth.h
//

/* This file contains the declaration for the base class JoinMethod.
 * Specific join methods are derived from this class.  The constructor
 * for any derived class must call the constructor for JoinMethod, and
 * an instance of each derived class must be created using a dummy
 * variable; this causes the instance of the derived class to be
 * linked into a list of available join methods so that the optimizer
 * can consider it during optimization.  The JoinMethod class consists
 * of the following members:
 *
 *      MakePlanNode                 creates a new PlanNode and inserts
 *                                   it into solution list (see sol_tbl.h);
 *                                   this is where cost estimates are 
 *                                   computed, as well as resulting tuple
 *                                   order
 *
 *      CanBeApplied                 returns true or false depending on
 *                                   whether the join method can be used
 *                                   given two PlanNodes to join and a 
 *                                   predicate
 *
 *      name                         stores the name of the join method,
 *                                   should be set in the constructor of
 *                                   the derived class
 *
 *      costDescription              stores a description of the cost 
 *                                   function that the join method uses
 *
 *      JoinMethod()                 constructor, responsible for linking
 *                                   the new instance of the class into
 *                                   the masterJoinMethodList
 *
 *  
 * Derived classes must define three member functions; a constructor,
 * CanBeApplied, and MakePlanNode.  CanBeApplied determines if two
 * PlanNodes could be joined using this JoinMethod class for a given
 * predicate.  The function MakePlanNode takes as its arguments two
 * PlanNodes, a list of join/select predicates, a primary join
 * predicate, a list of attributes to project on, and a solution list
 * to insert the new PlanNode into.  MakePlanNode creates a new
 * PlanNode, calculates the cost estimates and resulting tuple order
 * and inserts the new PlanNode into the given solution list.  Only
 * the Insert member function from SolutionList should be used for
 * inserting the new plan node.  Insert correctly handles pruning;
 * InsertHead, InsertTail, and InsertBeforeCurr do not.
 *
 * See nestloop (.h & .C) and sortmerge (.h & .C) for examples.
 */

#ifndef _JOINMETH_
#define _JOINMETH_
#include "planner.h"
#include "sol_tbl.h"
//#include "join.h"


class JoinMethod
{
protected:
    PlanNode *NewPlanNode(PlanNode *pn1, PlanNode *pn2, 
			  const SelectList & sl,
			  Select *primaryJoinPredicate,
			  const AttributeList & al);

    std::string name;
    std::string costDescription;

public:
    JoinMethod();
    virtual ~JoinMethod() { };
      		    	    
    std::string Name() { return name; };

    std::string CostDescription() { return costDescription; };

    virtual int CanBeApplied(PlanNode * /* pn1 */, PlanNode * /* pn2 */,
                                 Select * /* primaryJoinPredicate */) {return false;}

    virtual void MakePlanNode(PlanNode * /* pn1 */, PlanNode * /* pn2 */, 
			      const SelectList & /* sel */,
			      Select * /* primaryJoinPredicate */,
			      const AttributeList & /* al */,
                                  SolutionList * /* sol */) {}

//    virtual JoinFunction *GetJoinFun() = 0;

    // the == operator is defined because the List<JoinMethod>
    // class requires it
    int operator==(const JoinMethod & jm) { return (this == &jm); } ;
};

void ShowJoinMethods(int withCost = FALSE);

void ToggleJoinMethod(int num);

typedef DList<JoinMethod> JoinMethodListType;

// masterJoinMethodList is the list of all available join methods.
// It doesn't change during execution.
extern JoinMethodListType masterJoinMethodList;

/* activeJoinMethodList is the list of the join methods that are to be
 * considered during optimization.  It changes depending on user
 * specifications for which join methods are to be included and which
 * are to be excluded from the optimization process.  */
extern JoinMethodListType activeJoinMethodList;

#endif



