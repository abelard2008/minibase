#include "String.SLList.h"
#include "String.defs.h"
#include <iostream.h>



int main (void) {
	
	StringSLList a;

	cout << "Start empty? " << a.empty() << endl;

	a.append("bob");
	a.append("Billy");
	a.append("mack");
	a.append("Toledo Springs");
	a.prepend("Min");

	cout << "Length: " << a.length() << endl;

	Pix index;
	index = a.first();
	
	cout << "Contents:" << endl;
	for (Pix i = a.first(); i!=0; a.next(i))
	  cout << a(i) << endl;

	
	i = a.first();
	if (a(i)=="Min") {
		cout << "remove: " << a(i) << endl;
	    a.remove_front();
	}
	else {
		Pix j=i;
		for (a.next(j); j!=0; i=j,a.next(j)) {
			if (a(j)=="mack") {
				cout << "remove: " << a(j) << endl;
				a.del_after(i);
			}
		}
	}

	for (i = a.first(); i!=0; a.next(i))
	  cout << a(i) << endl;
}


