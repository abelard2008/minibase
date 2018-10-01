///////////////////////////////////////////////////////////////////////////
// Implementation of functional dependancies and their algorithms
//
// Implemented by Andrew Prock 1996
//
// Many of the algorithms used in this code are from "The Design of 
// Relational Databases" by Heikki Mannila and Kari-Jouko Raiha,
// Addison-Wesley 1992.
//
// Much of the code which this implmentation is based on the libg++
// library.  The library classes have been modified greatly and are
// distributed with the sources.  Any further distribution of this
// source code need follow the GNU Public Liscense.
///////////////////////////////////////////////////////////////////////////
#include "fdset.h"


FDset::FDset () : numFD(0), dirty(TRUE), canonical(FALSE)
///////////////////////////////////////////////////////////////////////////
// --
///////////////////////////////////////////////////////////////////////////
{
}

FDset::FDset (const FDset& F) : numFD(F.numFD), max(F.max), 
								dirty(F.dirty), canonical(F.canonical)
///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////
{
	for (int i=0; i<MAX_FD_NUM; i++)
	    FDlist[i] = F.FDlist[i];
}

FDset::~FDset ()
///////////////////////////////////////////////////////////////////////////
// --
///////////////////////////////////////////////////////////////////////////
{
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// MODIFICATION interface
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

int 
FDset::addFD (FD fd)
///////////////////////////////////////////////////////////////////////////
// Add an FD to the set
///////////////////////////////////////////////////////////////////////////
{
	int i;

	for (i=0; i< numFD; i++) {
		if (fd==FDlist[i])
		    return -1;
	}
	FDlist[numFD++] = fd;

	dirty = TRUE;  // for max
	return numFD-1;
}

int 
FDset::setLHS (const StrSet& lhs, int handle)
///////////////////////////////////////////////////////////////////////////
// set the lhs of the fd stored in "handle"
///////////////////////////////////////////////////////////////////////////
{
	dirty = TRUE;  // for max
	return FDlist[handle].setLHS(lhs);
}

int 
FDset::setRHS (const StrSet& rhs, int handle)
///////////////////////////////////////////////////////////////////////////
// set the rhs of the fd stored in "handle"
///////////////////////////////////////////////////////////////////////////
{
	dirty = TRUE;  // for max
	return FDlist[handle].setRHS(rhs);
}


int 
FDset::addFD (const StrSet& lhs, const StrSet& rhs)
///////////////////////////////////////////////////////////////////////////
// not tested
///////////////////////////////////////////////////////////////////////////
{
	FD f;
	f.setLHS( lhs );
	f.setRHS( rhs );

	return addFD(f);
}

int 
FDset::remFD (int handle)
///////////////////////////////////////////////////////////////////////////
// remove FD at handle index
///////////////////////////////////////////////////////////////////////////
{
	dirty = TRUE;  // for max
	if (handle >= numFD) 
	    return 0;		// failure
	FDlist[handle] = FDlist[--numFD];
	FDlist[numFD].clear();		// save space
	return 1;	// success
}

void 
FDset::clear ()
///////////////////////////////////////////////////////////////////////////
// clear all FDs
///////////////////////////////////////////////////////////////////////////
{
	for (int i=0; i<numFD; i++) {
		FDlist[i].clear();
	}
	numFD = 0;
	dirty = TRUE;  // for max
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// ACCESS functions
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

FD 
FDset::getFD (int handle) const
///////////////////////////////////////////////////////////////////////////
// ??
///////////////////////////////////////////////////////////////////////////
{
	return FDlist[handle];
}

StrSet 
FDset::getLHS (int handle) const
///////////////////////////////////////////////////////////////////////////
// kept for attrClosure
///////////////////////////////////////////////////////////////////////////
{
	return FDlist[handle].getLHS();
}

StrSet
FDset::getRHS (int handle) const
///////////////////////////////////////////////////////////////////////////
// kept for attrClosure
///////////////////////////////////////////////////////////////////////////
{
	return FDlist[handle].getRHS();
}

int 
FDset::count () const
///////////////////////////////////////////////////////////////////////////
// --
///////////////////////////////////////////////////////////////////////////
{
	return numFD;
}

void 
FDset::print () const
///////////////////////////////////////////////////////////////////////////
// --
///////////////////////////////////////////////////////////////////////////
{
	int i;
	for (i=0; i<numFD; i++) {
		cout << i << ": ";
		FDlist[i].print();
		cout << endl;
	}
	cout << endl;

//	lunion();
}

void 
FDset::print (int handle) const
///////////////////////////////////////////////////////////////////////////
// --
///////////////////////////////////////////////////////////////////////////
{
	FDlist[handle].print();
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// OVERLOADED OPERATORS
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

FDset&
FDset::operator= (const FDset& fdset)
///////////////////////////////////////////////////////////////////////////
// ??
///////////////////////////////////////////////////////////////////////////
{
	for (int i=0; i<fdset.numFD; i++) {
		FDlist[i] = fdset.FDlist[i];
	}
	numFD = fdset.numFD;
	max = fdset.max;
	dirty = fdset.dirty;
	canonical = fdset.canonical;

	return *this;
}

bool
FDset::operator== (const FDset& /*fd*/) const
///////////////////////////////////////////////////////////////////////////
// not implemented.  Should really return true if they are equivalent
///////////////////////////////////////////////////////////////////////////
{
	return 0; // should actually return success status
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// What attribs do we have anyway?
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

StrSet 
FDset::lunion () const
///////////////////////////////////////////////////////////////////////////
// this is the union of all the lhs attribs
///////////////////////////////////////////////////////////////////////////
{
	int i;
	Pix p;
	StringSLList lunion = setToList(FDlist[0].getLHS());
	StringSLList lhs;

	for (i=1; i<numFD; i++) {
		lhs = setToList(FDlist[i].getLHS());

		for (p=lhs.first(); p!=0; lhs.next(p)) {
			if (!lunion.contains(lhs(p))) {
				lunion.prepend(lhs(p));
			}
		}
	}

	return listToSet(lunion);
}

StrSet 
FDset::runion () const
///////////////////////////////////////////////////////////////////////////
// this is the union of all the rhs attribs
///////////////////////////////////////////////////////////////////////////
{

	int i;
	Pix j,k,add;
	StringSLList runion = setToList(FDlist[0].getRHS());
	StringSLList rhs;

	for (i=1; i<numFD; i++) {
		rhs = setToList(FDlist[i].getRHS());

		for (j=rhs.first(); j!=0; rhs.next(j)) {
			add = NULL;
			for (k=runion.first(); k!=0; runion.next(k)) {
				if ( runion(k)==rhs(j) ) {
					add = j;
				    break;
				}
			}
			if (add == NULL) {
				runion.prepend(rhs(j));
			}
		}
	}

	return listToSet(runion);
}

StrSet
FDset::univAttrs () const
///////////////////////////////////////////////////////////////////////////
// the union of lhs and rhs attribs
///////////////////////////////////////////////////////////////////////////
{
	StrSet l,r,u;

	l = lunion();
	r = runion();

	for(StrSet::iterator i=l.begin(); i!=l.end(); i++) {
		u.insert(*i);
	}
	for(StrSet::iterator i=r.begin(); i!=r.end(); i++) {
		u.insert(*i);
	}

	return u;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// library conversion utils
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

StrSet 
FDset::listToSet(const StringSLList l) const
///////////////////////////////////////////////////////////////////////////
// temporary conversion util
///////////////////////////////////////////////////////////////////////////
{
	StrSet s;

	for (Pix i=l.first(); i!=NULL; l.next(i)) {
		s.insert( l(i) );
	}

	return s;
}

StringSLList 
FDset::setToList(const StrSet s) const
///////////////////////////////////////////////////////////////////////////
// temporary conversion util
///////////////////////////////////////////////////////////////////////////
{
	StringSLList l;

	for (StrSet::iterator i=s.begin(); i!=s.end(); i++) {
		String tmp = *i;
		l.prepend(tmp);
	}

	return l;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// ALGORITHMS
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

StrSet
FDset::attrClosure (const StrSet& a) const
///////////////////////////////////////////////////////////////////////////
// see page 150 of Mannila and Raiha
// Note this code does not strictly follow their code.  It is very 
// nasty.
//
// one day I will convert all StringSLLists to StrSets
///////////////////////////////////////////////////////////////////////////
{
	int i,j;
	Pix p,q;
	StringSLList attribs = setToList(a);
	StringSLList attrc;
	int numAttribs;
	StringSLList lhs;
	StringSLList newAttrs;
	StringSLList newTmp;
	StringSLList univ = setToList(lunion());
	StringSLList rhs;

	intSLList aFDlist  [univ.length()];
	String    aFDname  [univ.length()];
	int       aFDcount [numFD];
	
	for (i=0; i<numFD; i++) 
	    aFDcount[i]=0;

	// foreach attribute in the union of LHS for the FD
	for (p=univ.first(),i=0; p!=0; univ.next(p),i++) {

		aFDname[i] = univ(p);

		// for each FD in FDset
		for (j=0; j<numFD; j++) {
			lhs = setToList(FDlist[j].getLHS());

			// if the lhs of the FD contains the ith attribute
			// in the union prepend that FD number (j) to the
			// set of FDs that have the attribute (i) in their lhs
			for (q=lhs.first(); q!=0; lhs.next(q)) {
				if ( univ(p)==lhs(q)) {
					aFDlist[i].prepend(j);
					aFDcount[j] = aFDcount[j]+1;
					break;
				}
			}
		}
	}
	numAttribs = i;
	Pix fd,attrp,nextq;
    int attr;

	// time to close attribute list
	// we will use the numbers
	newAttrs = attribs;

	while ( !newAttrs.empty() ) {

		// add new attributes to the list in the attribute closrure
		// note: they are instered in sorted order (nescessary!)
		for (p=newAttrs.first(); p!=0; newAttrs.next(p)) {
//			if (attrc.empty() || (String)(attrc.front()) > (String)(newAttrs(p)) ) {
			if (attrc.empty() || attrc.front() > newAttrs(p) ) {
				attrc.prepend(newAttrs(p));
			} else {
				for (q=attrc.first(); q!=0; attrc.next(q)) {
					nextq = q;
					attrc.next(nextq);
					if (nextq!=0) {
						if (attrc(nextq) > newAttrs(p)) {
							attrc.ins_after(q, newAttrs(p));
							break;
						} 
					} else {
						attrc.ins_after(q, newAttrs(p));
						break;
					}
				}
			}
		}

		newTmp.clear();

		for (attrp=newAttrs.first(); attrp!=0; newAttrs.next(attrp)) {
			
			// check to see if this attribute is in an antecedant
			attr = -1;
			for (i=0; i<numAttribs; i++) {
				if(aFDname[i]==newAttrs(attrp)) {
				    attr = i;
					break;
				}
			}
			if (attr == -1)
			  continue;

			// close
			for (fd=aFDlist[attr].first(); fd!=0; aFDlist[attr].next(fd)) {
				aFDcount[ aFDlist[attr](fd) ]= aFDcount[ aFDlist[attr](fd) ]-1;
				if (aFDcount[ aFDlist[attr](fd) ]==0) {
					rhs = setToList( getRHS(aFDlist[attr](fd)) );
					for(q=rhs.first(); q!=0; rhs.next(q)) {
						if (!attrc.contains(rhs(q))) {
							if (!newTmp.contains(rhs(q))) {
								newTmp.prepend(rhs(q));
							}
						}
					}
				}
			}
		}
		newAttrs = newTmp;
	}
			
	return listToSet(attrc);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// COVERS of FDset
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

FDset 
FDset::lminCover ()
///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////
{
		return *this; // to keep -Wall happy for now
}

FDset 
FDset::nonRedundantCover ()
///////////////////////////////////////////////////////////////////////////
// not tested
///////////////////////////////////////////////////////////////////////////
{
	FDset NR = *this;

	for (int i=numFD-1; i>=0; i-- ) {  // must iterate *down*
		NR.remFD(i);
		StrSet rhsOriginal = getLHS(i);
		StrSet lhsClosure = attrClosure(getLHS(i));
		if ( includes (rhsOriginal.begin(),rhsOriginal.end(),
					   lhsClosure.begin(),lhsClosure.end()) )
		{
			StrSet l=getLHS(i);
			StrSet r=getRHS(i);
			NR.addFD(l,r);
		}
	}
	return NR; // to keep -Wall happy for now
}

FDset 
FDset::canonicalCover ()
///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////
{
	return *this; // to keep -Wall happy for now
}

bool
FDset::losslessJoin (const StrSet& /*attribs*/) const
///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////
{
	return 0; // to keep -Wall happy for now
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// BCNF utils
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

list<int>
FDset::testBCNF (const StrSet& R) const
///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////
{
	list<int> violations;

	for (int i=numFD-1; i>=0; i--) {
		if (testBCNF(i,R)==FALSE) {
			violations.push_front(i);
		}
	}

	return violations;
}

bool 
FDset::testBCNF (int handle, const StrSet& R) const
///////////////////////////////////////////////////////////////////////////
// Tests whether fd in handle 
///////////////////////////////////////////////////////////////////////////
{
	return isKey (getLHS(handle),R);
}

bool
FDset::isKey (const Attrib& a) const
///////////////////////////////////////////////////////////////////////////
// not tested
///////////////////////////////////////////////////////////////////////////
{
	StrSet A;
	A.insert(a);

	return isKey(A, univAttrs());
}

bool
FDset::isKey (const StrSet& A, const StrSet& R) const
///////////////////////////////////////////////////////////////////////////
// not tested
///////////////////////////////////////////////////////////////////////////
{
	StrSet closure = attrClosure(A);

	if (includes(closure.begin(),closure.end(),
				 R.begin(),R.end() ) )
	{
		return TRUE;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// 3NF utils (note most of these *could* change max)
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

list<int>
FDset::test3NF (const StrSet& R) 
///////////////////////////////////////////////////////////////////////////
// not const because genMaxSets() could change max
///////////////////////////////////////////////////////////////////////////
{
	list<int> violations;

	for (int i=numFD-1; i>=0; i--) {
		if (test3NF(i,R)==FALSE) {
			violations.push_front(i);
		}
	}

	return violations;
}

bool
FDset::test3NF (int handle, const StrSet& R)
///////////////////////////////////////////////////////////////////////////
// not const because genMaxSets() could change max
///////////////////////////////////////////////////////////////////////////
{
	if (testBCNF(handle,R)==FALSE) {
		StrSet A = getRHS(handle);
		StrSet::iterator i=A.begin();
		Attrib a = (*i);
		if (isPrime(a,R)==FALSE) {
			return FALSE;
		}
	}
	
	return TRUE;
}

bool 
FDset::isPrime (const Attrib& a) 
///////////////////////////////////////////////////////////////////////////
// determines whether Attrib a is prime with respect to the FDset
///////////////////////////////////////////////////////////////////////////
{
	StrSet R = univAttrs();
	return isPrime (a,R);
}

bool 
FDset::isPrime (const Attrib& A, const StrSet& R) 
///////////////////////////////////////////////////////////////////////////
// could change max array 
// this is where the actual primality test is done
// A is the attribute which may be prime with respect to R
///////////////////////////////////////////////////////////////////////////
{
	genMaxSets();

	for (SetSet::iterator i = max[A].begin(); i!=max[A].end(); i++) {
	    StrSet s  = *i;
		s.insert(A);
		
		StrSet t = attrClosure(s);  //ugly
		if (t==R) {
			//cout << endl << A << " is prime" << endl;
			return TRUE;
		}
	}

	return FALSE; // to keep -Wall happy for now
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
// maximal set functions
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

bool 
FDset::maximal (StrSet Y, StrSet Yp, StrSet W) const
///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////
{
/*
	cout << "\n  Y in W? " << includes (W.begin(), W.end(),
				   Y.begin(), Y.end() ) << endl;
	cout << "  Yp in W? " << includes (W.begin(), W.end(),
				   Yp.begin(), Yp.end() ) << endl;
*/
	if (includes (W.begin(), W.end(),
				   Y.begin(), Y.end() ) &&
		!includes (W.begin(), W.end(),
				   Yp.begin(), Yp.end() ) )
	{
		return FALSE;
	}

	if (W.begin() == W.end() ) return FALSE;
	return TRUE;
}

bool
subsetIncludes(SetSet S, StrSet s) 
///////////////////////////////////////////////////////////////////////////
// in the alg on 243 of Manilla Raiha it calls for a "recursive" union
// even though it doesn't say so.  This is the workaround.
///////////////////////////////////////////////////////////////////////////
{
	for (SetSet::iterator i=S.begin(); i!=S.end(); i++) {
		if (includes ((*i).begin(), (*i).end(),
					  s.begin(), s.end()) )
		{
			return TRUE;
		}
	}
	return FALSE;
}

void 
outmax (MapSetSet M) 
///////////////////////////////////////////////////////////////////////////
// primarily for debugging
///////////////////////////////////////////////////////////////////////////
{
	ostream_iterator <String> out(cout, " ");
	for (MapSetSet::iterator i = M.begin(); i!=M.end(); i++) {
		cout << (*i).first << ": ";
		SetSet s = (*i).second;
		for (SetSet::iterator j = s.begin(); 
			 j!= s.end(); j++) 
		{
			cout << "{";
			copy ( (*j).begin(), (*j).end(), out);
			cout << "}, ";
		}
		cout << endl;
	}
}

bool 
FDset::genMaxSets ()
///////////////////////////////////////////////////////////////////////////
// see page 243 of Manilla Raiha
//
// note: this code could be more robust, should do more error and bounds
// checking.
///////////////////////////////////////////////////////////////////////////
{

	if (!dirty)          // only calculate if not done or set has changed
	    return TRUE;

	StrSet univ = univAttrs();
//	ostream_iterator <String> out(cout, " ");
//	StrSet tmpset;
//	SetSet tmp,tmp2;
	StrSet Y, Yprime;
//	SetSet Z;
	Attrib A;
	MapSetSet newmax;

	for (StrSet::iterator i=univ.begin(); i!= univ.end(); i++) {
		StrSet t;		t.insert (*i);
 		StrSet u;
		SetSet ss;

		set_difference (univ.begin(), univ.end(),
						t.begin(), t.end(),
						insert_iterator<StrSet> (u,u.begin()) );
		ss.insert(u);
		max[*i] = ss;
	}
//	outmax (max);

	FDset G;

	for (int i=0; i<numFD; i++) {
		Y = getLHS(i);
		Yprime = getRHS(i);
		G.addFD( FDlist[i] );

		// univ and R are the same (set of all atributes)
		for (StrSet::iterator j=univ.begin(); j!=univ.end(); j++) {
			A = *j;		// ok becuse A is a base type not a containter
			newmax[A] = max[A];  // will this work?  *must* by def see stl doc

			for (SetSet::iterator W=max[A].begin(); W!=max[A].end(); W++) {
/*
				cout << "Y : ";
				copy(Y.begin(), Y.end(), out);
				cout << "\nYp: ";
				copy(Yprime.begin(), Yprime.end(), out);
				cout << "\nW : " << A << ": ";
				copy((*W).begin(), (*W).end(), out);
				cout << "\n!Maximal?: ";
//				cout << maximal(Y,Yprime,*W); 
				cout << endl;
*/
				if (!maximal(Y,Yprime,*W)) {
					newmax[A].erase(*W);

					for (StrSet::iterator B=Y.begin(); B!=Y.end(); B++) {
						for (SetSet::iterator Z=max[*B].begin();
							 Z!=max[*B].end(); Z++)
						{
							StrSet result;  // used to store (W intersect Z)
							set_intersection((*W).begin(), (*W).end(),
											 (*Z).begin(), (*Z).end(),
											 insert_iterator<StrSet> 
											 (result, result.begin()) );

							if (!subsetIncludes(newmax[A], result)) {
/*
								cout << "## ---- B: " << *B << endl;
								cout << "## Y : ";
								copy(Y.begin(), Y.end(), out);
								cout << "\n## Yp: ";
								copy(Yprime.begin(), Yprime.end(), out);
								cout << "\n## WintZ - " << A << ": ";
								copy(result.begin(), result.end(), out);
								cout << "\n## Maximal?: ";
//								cout << maximal(Y,Yprime,result); 
								cout << endl;
*/							
								if (maximal(Y,Yprime,result) ) {
//									cout << "## INSERTED into: " << A
//									     <<  endl << endl;
									newmax[A].insert(result);
								}
							}						  
						}
					}
				}
			}
		}
		max = newmax;
//		cout << endl;
//		outmax(max);
//		cout << "-----------------------" << endl;
	}

//	cout << "genMaxSets is not fully implemeted" << endl;
//	outmax (max);

	dirty = FALSE;
	return TRUE; // to keep -Wall happy for now
}


