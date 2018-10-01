#ifndef ITERATOR_HDR
#define ITERATOR_HDR

#include <ostream>
#include "tuple.h"
#include "minirel.h"
#include "new_error.h"

enum RelSpec {outer, innerRel};  // changed by BK from inner to innerRel

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
	virtual Status get_next(Tuple*& tuple) = 0;
	virtual ~Iterator(){};

//#ifdef TEST
Status get_buffer_pages(int n_pages, int *PageIds, char **bufs);
Status free_buffer_pages(int n_pages, int *PageIds );
//#endif


};


#endif
