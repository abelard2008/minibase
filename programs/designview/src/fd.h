#ifndef _FD_H
#define _FD_H

#include <bool.h>
#include <stl.h>
#include "String.h"

typedef String Attrib;
typedef set<String, less<String> > StrSet;

class FD {

public:
	FD();
	FD(const FD& fd);
	~FD();

	int addLHS (const Attrib& a);
	int addRHS (const Attrib& a);
	int remLHS (const Attrib& a);
	int remRHS (const Attrib& a);
	int setLHS (const StrSet& attribs);
	int setRHS (const StrSet& attribs);

	StrSet getLHS () const;
	StrSet getRHS () const;
	int countRHS () const;
	int countLHS () const;
	int size () const;
	void print () const;
	void clear ();

	FD& operator=(const FD& fd);
	bool operator==(const FD& fd) const;

private:
	StrSet rhs;
	StrSet lhs;
	int rhsLen;
	int lhsLen;
};



#endif
