#ifndef _FDset_h
#define _FDset_h

#include <bool.h>
#include <stl.h>
#include "fd.h"
#include "String.SLList.h"
#include "int.SLList.h"

typedef set<String, less<String> > StrSet;
typedef set<StrSet, less<StrSet> > SetSet;
typedef map<String, SetSet, less<String> > MapSetSet;
const int MAX_FD_NUM = 100;

class FDset {

public:
	FDset();
	FDset(const FDset& fdset);
	~FDset();

	int addFD (FD fd);
	int addFD (const StrSet& lhs, const StrSet& rhs);
	int setLHS (const StrSet& lhs, int handle);
	int setRHS (const StrSet& rhs, int handle);

	StrSet getLHS (int handle) const;
	StrSet getRHS (int handle) const;

	FD getFD (int handle) const;
	int remFD (int handle);
	int count () const;
	void clear ();

	void print () const;
	void print (int handle) const;

	FDset& operator= (const FDset& fd);     // overloaded assignment
	bool operator== (const FDset& fd) const; // comparison

	StrSet lunion () const;     // union of left hand sides
	StrSet runion () const;     // union of right hand sides
	StrSet univAttrs () const;        // union of all attribs in FDset

	StrSet attrClosure (const StrSet& attribs) const;
	
	// various FDset cover utilities
	FDset lminCover ();
	FDset nonRedundantCover ();
	FDset canonicalCover ();

	// not yet implemented
	bool losslessJoin (const StrSet& attribs) const;

	// 3NF tests
	list<int> test3NF (const StrSet& R);
	// check FDset (returns violations)
	bool test3NF (int handle, const StrSet& R);
	// check single FD
	bool isPrime (const Attrib& A);
	// check attrib for primality
	bool isPrime (const Attrib& A, const StrSet& R);		

	// BCNF test
    // check FDset (returns viols)
	list<int> testBCNF (const StrSet& R) const;	
	// check single FD
	bool testBCNF (int handle, const StrSet& R) const;
	// check if attrib is a key
	bool isKey (const Attrib& a) const;
	// check if set of attribs is a key
	bool isKey (const StrSet& A, const StrSet& R) const;

private:
	FD FDlist[MAX_FD_NUM];
	int numFD;
	MapSetSet max;
	bool dirty;
	bool canonical;

	// private methods
	bool genMaxSets();
	bool maximal(StrSet ,StrSet,StrSet) const;
	StrSet listToSet(const StringSLList l) const;
	StringSLList setToList(const StrSet s) const;
};



#endif
