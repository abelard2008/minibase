#include <assert.h>
#include "planner.h"
#include "interpreter.h"
#include "iterator.h"
#include "joinmeth.h"
#include "joins.h"
#include "query.h"
#include "new_error.h"
#include "file_scan.h"
#include "index_scan.h"


enum {
    NEED_LEFT_CHILD,
    NEED_RIGHT_CHILD,
    CHILD_MUST_BE_PLAN,
};

static const char* errorMessages[] = {
    "PlanNode must have left child",
    "PlanNode must have right child",
    "Child of the join method must be PlanNode",
};

static error_string_table errors( PLANNER, errorMessages );

#define MY_ASSERT(A, B) \
	if(!A){ \
		status = MINIBASE_FIRST_ERROR( PLANNER, B ); \
		return NULL; \
	}

#define CHECK(A) if(A == PLANNER)\
		return NULL;\
	if(A != OK){		 \
                status = MINIBASE_CHAIN_ERROR( PLANNER, A );\
		return NULL;\
	}

	
Iterator* PlanNode::interpret(AttributeList& attrs, Status& status){

	FldSpec* projects;
	CondExpr** selects;
	AttrType* types;
//	Attribute* attrib;
	int noInFlds;
	int noOutFlds;
	std::string amName;	
	std::string relName;
	std::string indName;
	AttributeList al;

	attrs = project;	// value to be returned 

	children.GoHead();
	MY_ASSERT( children.CurrPtr(), NEED_LEFT_CHILD );
	if(children.CurrPtr()->IsPlanNode()){

		// This is a join method

		FldSpec* rprojects;
		CondExpr** rselects;
		AttrType* rtypes;
		AttrType* ltypes;
		short* rStrLens;
		short* lStrLens;
//		Attribute* rattrib;
		int rInFlds;
		int lInFlds;
		int rOutFlds;
		int noResFlds;
		int memory;
		PlanNode* leftChild = children.CurrPtr()->GetPlanNode();
		MY_ASSERT( leftChild, CHILD_MUST_BE_PLAN );
		children.GoNext();
		MY_ASSERT( children.CurrPtr(), NEED_RIGHT_CHILD );
		PlanNode* rightChild = children.CurrPtr()->GetPlanNode();
		MY_ASSERT( rightChild, CHILD_MUST_BE_PLAN );
		AttributeList rightAl;
		AttributeList leftAl;
		Iterator* leftIter;
		Iterator* rightIter;
		int innerCol, outerCol;

		leftIter = leftChild->interpret(leftAl, status);
		CHECK(status);

		// BK => This if is not good...  Done to be compatible with new/old optimizer
		if((joinMethod->Name() == "Nested Loops with Right SubTree as Inner") ||
		   (joinMethod->Name() == "Tuple Oriented Nested Loops with Right SubTree as Inner") ||
		   (joinMethod->Name() == "Page Oriented Nested Loops with Right SubTree as Inner") ||
		   (joinMethod->Name() == "Index Nested Loops with Right SubTree as Inner")){

                        int field_number;   // BK -  used to find out what field
					    // an index is on
                        int RightindexOnly;      // BK - used if index only scan
			rightChild->LeafInfo(amName, relName, indName, 
					     rightAl, field_number, RightindexOnly);
			RightindexOnly = indexOnly;
			Converter rightConv(rightChild, rightAl);	

			rightConv.get(rInFlds, rtypes, rStrLens, 
				rselects, rprojects, rOutFlds);
			Converter thisConv(this, leftAl, rightAl);

			if(amName == "FileScan"){
				thisConv.get(memory, lInFlds, ltypes, 
					lStrLens, selects, projects, noResFlds);
				return new nested_loops_join(ltypes, lInFlds, lStrLens,
					rtypes, rInFlds, rStrLens, memory, leftIter, 
					relName,  selects, rselects,
					 projects, noResFlds, status);
			}
			else if(amName == "B_Index"){
				thisConv.getINL(memory, lInFlds, ltypes, lStrLens, 
					selects, projects, noResFlds, outerCol, innerCol);
				return new index_nested_loop(ltypes, lInFlds, lStrLens,
					rtypes, rInFlds, rStrLens, outerCol, innerCol,
					memory, leftIter, B_Index, indName, relName,
					selects, rselects, projects, noResFlds, 
					RightindexOnly, status);
			}
			else if(amName == "Hash"){
				thisConv.getINL(memory, lInFlds, ltypes, lStrLens, 
					selects, projects, noResFlds, outerCol, innerCol);
				return new index_nested_loop(ltypes, lInFlds, lStrLens,
					rtypes, rInFlds, rStrLens, outerCol, innerCol,
					memory, leftIter, Hash, indName, relName,
					selects, rselects, projects, noResFlds, 
					RightindexOnly, status);
			}
			else{
				std::cout << "Unknown access method: " << amName << std::endl;
				assert(0);
			}
		}
		else if(joinMethod->Name() == "Sort Merge"){

			int outerFldLen;
			int innerFldLen;
			TupleOrder outerOrd, innerOrd;

			projects = new FldSpec[MAX_ATTR_LIST];
			selects = new CondExpr*[MAX_ATTR_LIST];
			ltypes = new AttrType[MAX_ATTR_LIST];
			rtypes = new AttrType[MAX_ATTR_LIST];
			lStrLens = new short[MAX_ATTR_LIST];
			rStrLens = new short[MAX_ATTR_LIST];

			rightIter = rightChild->interpret(rightAl, status);
			CHECK(status);
			Converter thisConv(this, leftAl, rightAl);
			lInFlds = thisConv.convert(ltypes, lStrLens, outer);
			rInFlds = thisConv.convert(rtypes, rStrLens, innerRel);
			memory = MemoryRequired();
			thisConv.convert(predicate, selects, outerCol, innerCol);
			noResFlds = thisConv.convert(project, projects);
			thisConv.convert(outer, leftChild, outerCol, outerFldLen, 
				outerOrd);
			thisConv.convert(innerRel, rightChild, innerCol, innerFldLen, 
				innerOrd);
			TupleOrder sortOrd = accessMethod.Order();
			bool lSorted, rSorted;

			if(sortOrd != Random){
				lSorted = (outerOrd == sortOrd);
				rSorted = (innerOrd == sortOrd);
			}
			else if(outerOrd != Random){
				sortOrd = outerOrd;
				lSorted = true;
				rSorted = (innerOrd == sortOrd);
			}
			else if(innerOrd != Random){
				sortOrd = innerOrd;
				lSorted = false;
				rSorted = true;
			}
			else{
				sortOrd = Ascending;
				lSorted = false;
				rSorted = false;
			}

			return new sort_merge(ltypes, lInFlds, lStrLens,
				rtypes, rInFlds, rStrLens, outerCol, outerFldLen,
				innerCol, innerFldLen, memory, leftIter, 
				rightIter, lSorted, rSorted, sortOrd,
				selects, projects, noResFlds, status);
		}
		else {
			std::cout << "Unknown join method: " << joinMethod->Name() << std::endl;
			std::cout << "See help pages\n";
			assert(0);
		}
	}
	else {
		
		short* strLens;

		// This is plan node next to leaf

                int field_number;   // BK -  used to find out what field
				    // an index is on
                int RightindexOnly;      // BK - used if index only scan
		LeafInfo(amName, relName, indName, al, field_number, RightindexOnly);

		Converter converter(this, al);
		converter.get(noInFlds, types, strLens, selects, 
			projects, noOutFlds);

		if(amName == "FileScan"){
			return new FileScanIter( relName, types, strLens, 
				noInFlds, noOutFlds, projects, selects, status);
		}
		else if(amName == "B_Index"){

			return new IndexScanIter(B_Index, relName,
				indName, types, strLens, 
				noInFlds, noOutFlds, projects, selects, 
				field_number, RightindexOnly, status);   // BK
		}
		else if(amName == "Hash"){

			return new IndexScanIter(Hash, relName,
				indName, types, strLens, 
				noInFlds, noOutFlds, projects, selects, 
				field_number, RightindexOnly, status);       // BK
		}
		else {
			std::cout << "Unknown access method: " << amName << std::endl;
			assert(0);
		}
	}
	return 0;
}

void PlanNode::LeafInfo(
	std::string& amName, 
	std::string& relName, 
	std::string& indName, 
	AttributeList& al,
	int& fldNum,
	int& index_only){

	Plan_AccessMethod* am;
	children.GoHead();
	am = children.CurrPtr()->GetPlan_AccessMethod();
	assert(!joinMethod);	// cannot be join

	Attribute* attrib = am->AttributePtr();

	amName = am->Name();	
		// Can be : "FileScan" ...

	relName = attrib->RelationPtr()->OriginalName();
	if(am->IndexPtr()){
		indName = am->IndexPtr()->Filename();
	}
	else{
		indName = "";
	}
	al = attrib->RelationPtr()->attributeList;
	fldNum = attrib->AttrID();
	index_only = indexOnly;
}

void PlanNode::ShowRecursive(){
	Show();
  	Child* childPtr;
  	for (children.GoHead(); (childPtr = children.CurrPtr()); children.GoNext()){
    		if (childPtr->IsPlanNode()){
			childPtr->GetPlanNode()->ShowRecursive();
		}
    		else{
			childPtr->GetPlan_AccessMethod()->Show();
		}
	}
}
