#ifndef ITERATOR_HDR
#define ITERATOR_HDR

#include <ostream>
#include "tuple.h"
#include "query.h"

enum RelSpec {outer, inner};

struct FldSpec {
	RelSpec relation;
	int offset;
};

std::ostream& operator<<(std::ostream& out, FldSpec fld);

struct CondExpr {

// This structure will hold single select condition
// It is an element of linked list which is logically connected by OR operators

	AttrOperator op;	// Operator like "<"
	AttrType type1;
	AttrType type2;	// Types of operands 

	// Null AttrType means that operand is not a literal but an attribute name

	union {
		FldSpec symbol;
		char* string;
		int integer;
		float real;
	} operand1, operand2;

	CondExpr* next;	// Pointer to the next element in linked list
};

std::ostream& operator<<(std::ostream& out, CondExpr* expr);
	
class Iterator {

	// All the relational operators and access methods are iterators.

public:
	virtual Status get_next(Tuple*& tuple){};
	virtual ~Iterator(){};

};

class FileScanIter : public Iterator {

// File Scan iterator will sequentially scan the file and perform
// selections and projections.

public:
	FileScanIter(
		char* relName, 	// Name of the input relation
		AttrType types[],	// Array of types in this relation
		short* str_sizes,	// Array of string sizes
		int noInFlds,		// Number of fields in input tuple 
		int noOutFlds,		// Number of fields in output tuple
		FldSpec outFlds[],	// Fields to project
		CondExpr* selects[],	// Conditions to apply
		Status& s
	);

	Status reset(){};

	// Reset is needed in nested loop join for the inner relation.

	Status get_next(Tuple*& tuple){}
	~FileScanIter(){}
};

class IndexScanIter : public Iterator {

// Index Scan iterator will directly access the required tuple using
// the provided key. It will also perform selections and projections.

public:
	IndexScanIter(
		IndexType index,	// Type of the index
		char* relName, 	// Name of the input relation
		char* indName, 	// Name of the input index 
		AttrType types[],	// Array of types in this relation
		short* str_sizes,	// Array of string sizes
		int noInFlds,		// Number of fields in input tuple 
		int noOutFlds,		// Number of fields in output tuple
		FldSpec outFlds[],	// Fields to project
		CondExpr* selects[],	// Conditions to apply
		Status& s
	);

	Status get_next(Tuple*& tuple){}
	~IndexScanIter(){}
};

#endif
