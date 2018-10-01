#ifndef ITERATOR_HDR
#define ITERATOR_HDR

#include "/u/students/d/donjerko/public/html/tuple.h"
#include "query.h"

enum RelSpec {inner, outer};

struct FldSpec {
	RelSpec relation;
	int offset;
};


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

class Iterator {

	// All the relational operators and access methods are iterators.

public:
	virtual Status get_next(Tuple*& tuple) = 0;
	virtual ~Iterator() = 0;

};

class FileScanIter : public Iterator {

// File Scan iterator will sequentially scan the file and perform
// selections and projections.

public:
	FileScanIter(
		char* relName, 	// Name of the input relation
		AttrType types[],		// Array of types in this relation
		int noInFlds,		// Number of fields in input tuple 
		int noOutFlds,		// Number of fields in output tuple
		FldSpec outFlds[],		// Fields to project
		CondExpr* selects[]	// Conditions to apply
	);

	Status reset();

	// Reset is needed in nested loop join for the inner relation.

	Status get_next(Tuple*& tuple);
	~FileScanIter();
};

class IndexScanIter : public Iterator {

// Index Scan iterator will directly access the required tuple using
// the provided key. It will also perform selections and projections.

public:
	IndexScanIter(

	// This will actually be either B_TreeIter or HashIter

		char* relName, 	// Name of the input relation
		char* indName, 	// Name of the input index 
		AttrType types[],		// Array of types in this relation
		int keyPos,		// Position of the key in input tuple 
		int noInFlds,		// Number of fields in input tuple 
		int noOutFlds,		// Number of fields in output tuple
		FldSpec outFlds[],		// Fields to project
		CondExpr* selects[]	// Conditions to apply
	);

	Status get_next(Tuple*& tuple);
	~IndexScanIter();
};

class SortIter : public Iterator {

// It seems that the present version of optimizer will never call sort routine.
// If this remain to be true we don't need this iterator.

public:

	SortIterator(
		Iterator* inIter,		// Input iterator
		AttrType types[],			// Types of the input iterator 
		int noInFlds,			// Number of fields in input tuple 
		int noOutFlds,			// Number of fields in output tuple
		FldSpec outFlds[],			// Fields to project
		CondExpr* selects[]		// Conditions to apply
	);

	Status get_next(Tuple*& tuple);
	~SortIter();
};		

class SMJoinIter : public Iterator {


public:

	SMJoinIter(
		Iterator* outerIter,	// Outer iterator
		Iterator* innerIter,	// Inner iterator
		TupleOrder outer,		// order of the join column
		TupleOrder inner,		// order of the join column
		int outerCol,			// Outer join column
		int innerCol,			// Inner join column
		AttrType outType[],		// Types of the outer iterator 
		AttrType inType[],			// Types of the inner iterator 
		int noInnerFlds,		// Number of fields in inner tuple 
		int noOuterFlds,		// Number of fields in outer tuple
		FldSpec outFlds[],		// Fields to project
		int noProjFlds,			// Number of project fields
		CondExpr* selects[]		// Conditions to apply
	);

	Status get_next(Tuple*& tuple);
	~SMJoinIter();
};		

class SNLJoinIter : public Iterator {

// The same interface would be used for Block nested loop join.

public:

	SNLJoinIter(
		Iterator* outerIter,	// Outer iterator
		Iterator* innerIter,	// Inner iterator
		int outerCol,			// Outer join column
		int innerCol,			// Inner join column
		AttrType outType[],		// Types of the outer iterator 
		AttrType inType[],			// Types of the inner iterator 
		int noInnerFlds,		// Number of fields in inner tuple 
		int noOuterFlds,		// Number of fields in outer tuple
		FldSpec outFlds[],		// Fields to project
		int noProjFlds,			// Number of project fields
		CondExpr* selects[]		// Conditions to apply
	);

	Status get_next(Tuple*& tuple);
	~SNLJoinIter();
};		

class HashJoinIter : public Iterator {

public:

	HashJoinIter(
		Iterator* outerIter,	// Outer iterator
		Iterator* innerIter,	// Inner iterator
		int outerCol,			// Outer join column
		int innerCol,			// Inner join column
		AttrType outType[],		// Types of the outer iterator 
		AttrType inType[],			// Types of the inner iterator 
		int noInnerFlds,		// Number of fields in inner tuple 
		int noOuterFlds,		// Number of fields in outer tuple
		FldSpec projFlds[],		// Fields to project
		int noProjFlds,			// Number of project fields
		CondExpr* selects[]		// Conditions to apply
	);

	Status get_next(Tuple*& tuple);
	~HashJoinIter();
};		

class INLJoinIter : public Iterator {

// Index nested loop join cannot use the IndexScanIter.
// This means that this join will behave like the unary iterator, i.e.
// it will take only one tuple as an input.

public:

	INLJoinIter(
		//------------------------- Index arguments
		char* relName,			// Name of the inner relation
		char* indName,			// Name of the index on inner relation
		AttrType inType[],			// Types of the inner relation 
		int noInFlds,			// Number of fields in input tuple 
		int noOutFlds,			// Number of fields in output tuple
		FldSpec outFlds[],			// Fields to project
		CondExpr* indSelects[],	// Conditions to apply on index access
		//------------------------- join arguments, inner rel is index
		Iterator* outerIter,	// Outer iterator
		int innerCol,			// Inner join column
		int outerCol,			// Outer join column
		AttrType outType[],		// Types of the outer iterator 
		int noOuterFlds,		// Number of fields in outer tuple
		FldSpec outFlds[],		// Fields to project
		int noProjFlds,			// Number of project fields
		CondExpr* selects[]		// Conditions to apply after the join
	);

	Status get_next(Tuple*& tuple);
	~INLJoinIter();
};		

#endif
