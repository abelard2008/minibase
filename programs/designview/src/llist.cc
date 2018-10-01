#include "int.SLList.h"
#include "int.defs.h"
#include <iostream.h>


int main (void) {
	
	intSLList a;

	cout << "Start empty? " << a.empty() << endl;

	a.append(4);
	a.append(3);
	a.append(2);
	a.append(1);
	a.prepend(0);

	cout << "Length: " << a.length() << endl;

	Pix index;
	index = a.first();
	
	cout << "Contents:" << endl;
	for (Pix i = a.first(); i!=0; a.next(i))
	  cout << a(i) << endl;
}
