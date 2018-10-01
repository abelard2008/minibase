#ifndef INTERPRETER_HDR
#define INTERPRETER_HDR

#include <assert.h>
#include <string.h>
#include "query.h"
#include "planner.h"
#include "iterator.h"

const int MAX_ATTR_LIST = 100;     // Maximum length of attribute list

class Converter {

// Converts various data structures that contains Attributes into the
// analagous structures that contain field specification. (pair of numbers)

public:

	Converter(PlanNode* plan, AttributeList al)
		: plan(plan), al1(al), unary(true) {};
	// Converter constructor takes PlanNode on which this conversion is to be
	// done and the attribute list of the CHILD node (this is unary converter).

	Converter(PlanNode* plan, AttributeList al1, AttributeList al2) 
		: plan(plan), al1(al1), al2(al2), unary(false) {};
	// Converter constructor takes PlanNode on which this conversion is to be
	// done and the attribute list of the two of its CHILDS.
	// (this is binary converter).

	int convert(AttributeList project, FldSpec outFlds[]);
	// This function will convert attribute list into the field specifications.

	void convert(Attribute* attrib, FldSpec& fld);
	// This function will convert single attribute into field specifications.

	CondExpr* convert(SelectElem* selEl, CondExpr* next);
	// Takes SelectElem and next ptr and returns CondExpr

	CondExpr* convert(SelectTerm* selTm, CondExpr* next);
	// Takes SelectTerm and next ptr and returns CondExpr

	CondExpr* convert(SelectExpr* selEx, CondExpr* next);
	// Takes SelectExpr and next ptr and returns CondExpr

	CondExpr* convert(Select sel, CondExpr* next){
		// Converts single Select into the CondExpr which is return value
		// of the function
		return convert(sel.GetExpr(), next);
	}

	int convert(SelectList selLst, CondExpr* selects[]);
	// Convert Select List into the linked list of CondExpr.
	// Returns the length of the list.

	int convert(SelectList selLst, CondExpr* selects[], int& outerColl, 
		int& innerColl);
	// Convert Select List into the linked list of CondExpr but in
	// addition isolates the primary join predicate (which is assumed
	// to be equality) and expresses it in terms of outer and inner 
	// column naumbers. Returns the length of the list.

	int convert(AttrType types[], short* strLens, RelSpec rel = outer);
	// This will return array of types and the string lengths for the
	// Attribute list given in constructor.

	void convert(RelSpec rel, PlanNode* child, int column,
		int& fldLen, TupleOrder& ord);
	// This is the function specific for the sort merge join.
	// It returns the field length and tuple order of the child on the
	// specified column.

	void get(int& noInFlds, AttrType*& types, short*& strLens, 
		CondExpr**& selects, FldSpec*& projects, int& noOutFlds);
	// This function will return all the relevant information for the
	// unary iterator.

	void get(int& memory, int& noOuterFlds, 
		AttrType*& outerTypes, short*& outerStrLens, CondExpr**& selects, 
		FldSpec*& projects, int& noResFlds);
	// This will return all the relevant information for simple nested
	// loop join.

	void getINL(
	     int& memory, int& noOuterFlds,
		AttrType*& outerTypes, short*& outerStrLens, CondExpr**& selects,
		FldSpec*& projects, int& noResFlds, int& outerCol, int& innerCol);
	// This will return all the relevant information for index nested
	// loop join.
	
private:

	PlanNode* plan;
	AttributeList al1, al2;
	bool unary;
};

#endif
