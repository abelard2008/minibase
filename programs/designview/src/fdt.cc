#include <iostream.h>
#include "fdset.h"

int main (void) {

	FD f;
	FDset F;

	String a = "a";
	String b = "b";
	String c = "c";
	String d = "d";
	String e = "e";

	StrSet A;
	StrSet B;
/*	
	A.add(a); A.add(b); A.add(c);
	B.add(d); B.add(b); B.add(c);

	StrSet D = A&B;
	StrSet E = A|B;
	cout << "Intersect: " << D << "Union: " << E;
	*/
	f.clear();
	f.addLHS(a);
	f.addLHS(b);
	f.addRHS(d);
	F.addFD(f);

	f.clear();
	f.addLHS(b);
	f.addLHS(c);
	f.addRHS(d);
	F.addFD(f);

	f.clear();
	f.addLHS(d);
	f.addRHS(e);
	F.addFD(f);

	f.clear();
	f.addLHS(e);
	f.addRHS(a);
	F.addFD(f);

	F.print();
	cout << "Is A prime?: " << F.isPrime(d) << endl;
	
}



