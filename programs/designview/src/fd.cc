#include "fd.h"


FD::FD () : rhsLen(0), lhsLen(0)
///////////////////////////////////////////////////////////////////////////
// Constructor, makes sure the lists are clear and the counts are zero.
///////////////////////////////////////////////////////////////////////////
{
//	lhs.erase();
//	rhs.clear();
}

FD::FD (const FD& fd)
///////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////
{
	lhsLen = fd.lhsLen;
	rhsLen = fd.rhsLen;
	lhs = fd.lhs;
	rhs = fd.rhs;
}

FD::~FD ()
///////////////////////////////////////////////////////////////////////////
// Place Holder
///////////////////////////////////////////////////////////////////////////
{
}

int FD::addRHS (const Attrib& a) 
///////////////////////////////////////////////////////////////////////////
// Add an attribute to the right hand side.
///////////////////////////////////////////////////////////////////////////
{
	rhs.insert(a);
	rhsLen++;
	return 1; // should be status
}

int FD::addLHS (const Attrib& a)
///////////////////////////////////////////////////////////////////////////
// Add an attribute to the right hand side.
///////////////////////////////////////////////////////////////////////////
{
	lhs.insert(a);
	lhsLen++;
	return 1; // should be status
}

int FD::setLHS (const StrSet& attribs)
///////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////
{
	lhs = attribs;
	return 1; // should be status
}

int FD::setRHS (const StrSet& attribs)
///////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////
{
	rhs = attribs;
	return 1; // should be status
}

int FD::remRHS (const Attrib& a)
///////////////////////////////////////////////////////////////////////////
// Removes specified attribute from rhs.
///////////////////////////////////////////////////////////////////////////
{
/*	
	Pix i,j;

	i = rhs.first();
	if ( rhs(i) == a ) {
		cout << "remove: " << rhs(i) << endl;
	    rhs.remove_front();
	}
	else {
		j=i;
		for (rhs.next(j); j!=0; i=j,rhs.next(j)) {
			if ( rhs(j) == a ) {
				cout << "remove: " << rhs(j) << endl;
				rhs.del_after(i);
			}
		}
	}
*/
	rhs.erase(a);
	rhsLen--;
	return 1;
}

int FD::remLHS (const Attrib& a)
///////////////////////////////////////////////////////////////////////////
// Removes specified attribute from rhs.
///////////////////////////////////////////////////////////////////////////
{
/*
	Pix i,j;
	
	i = lhs.first();
	if ( lhs(i) == a ) {
		cout << "remove: " << lhs(i) << endl;
	    rhs.remove_front();
	}
	else {
		j=i;
		for (lhs.next(j); j!=0; i=j,lhs.next(j)) {
			if ( lhs(j) == a ) {
				cout << "remove: " << lhs(j) << endl;
				lhs.del_after(i);
			}
		}
	}
*/
	lhs.erase(a);
	lhsLen--;
	return 1;
}

StrSet FD::getRHS () const
///////////////////////////////////////////////////////////////////////////
// Returns a copy of the linked list that represents the right hand
// side attributes.
///////////////////////////////////////////////////////////////////////////
{
    StrSet rhscopy = rhs;
    return rhscopy;
}

StrSet FD::getLHS () const
///////////////////////////////////////////////////////////////////////////
// Returns a copy of the linked list that represents the left hand
// side attributes.
///////////////////////////////////////////////////////////////////////////
{
    StrSet lhscopy = lhs;
    return lhscopy;
}

int FD::countRHS () const
///////////////////////////////////////////////////////////////////////////
// Returns the number of attributes in the right hand side of the FD.
///////////////////////////////////////////////////////////////////////////
{
	return rhsLen;
}

int FD::countLHS () const
///////////////////////////////////////////////////////////////////////////
// Returns the number of attributes in the left hand side of the FD.
///////////////////////////////////////////////////////////////////////////
{
	return lhsLen;
}

int FD::size () const
///////////////////////////////////////////////////////////////////////////
// Returns the total number of attributes in FD
///////////////////////////////////////////////////////////////////////////
{
	return lhsLen+rhsLen;
}

void FD::clear () 
///////////////////////////////////////////////////////////////////////////
// Returns the total number of attributes in FD
///////////////////////////////////////////////////////////////////////////
{
	rhs.erase(rhs.begin(),rhs.end());
	lhs.erase(lhs.begin(),lhs.end());
	rhsLen = 0;
	lhsLen = 0;
}

void FD::print () const
///////////////////////////////////////////////////////////////////////////
// Prints out an FD to standard out.
///////////////////////////////////////////////////////////////////////////
{
/*
    Pix i;

    for (i=lhs.first(); i!=0; lhs.next(i)) 
	cout << "{" << lhs(i) << "}" << " ";
    cout << "-> ";
    for (i=rhs.first(); i!=0; rhs.next(i)) 
	cout << "{" << rhs(i) << "}" << " ";
    cout << endl;
*/
	ostream_iterator <String> out(cout, " ");
	copy (lhs.begin(), lhs.end(), out);
	cout << "-> ";
	copy (rhs.begin(), rhs.end(), out);
}

bool FD::operator==(const FD& fd1) const
///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////
{
/*
	Pix i,j;

	if (rhs.length() != fd1.rhs.length() || lhs.length() != fd1.lhs.length())
	    return 0;
	for (i=lhs.first(),j=fd1.lhs.first(); i!=0; lhs.next(i),fd1.lhs.next(j))
		if( lhs(i) != fd1.lhs(j) )
		    return 0;
	for (i=rhs.first(),j=fd1.rhs.first(); i!=0; rhs.next(i),fd1.rhs.next(j))
		if( rhs(i) != fd1.rhs(j) )
		    return 0;
	return 1;
*/
	if (rhs==fd1.rhs && lhs==fd1.lhs)
	    return TRUE;
	return FALSE;
}

FD& FD::operator=(const FD& fd1)
///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////
{
	rhs = fd1.rhs;
	lhs = fd1.lhs;
	rhsLen = fd1.rhsLen;
	lhsLen = fd1.lhsLen;
	return *this;
}
