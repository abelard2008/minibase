#include <ostream>
#include <string.h>
#include "error.h"
#include "tuple.h"

main(){
	Error err;
	int integer;
	char* name = "donko.";
	float dec = 2.345;
	short str[] = {100,100,100};
	AttrType types[] = {
		attrInteger, attrString, attrReal, 
		attrInteger, attrString, attrReal, attrInteger};
	Tuple* tup = (Tuple*) new char[Tuple::max_size];
     err.print(tup->setHdr(7, types, str));
	std::cout << "number of fields in this tuple " << tup->noOfFlds() << endl;
	std::cout << "size of this tuple is " << tup->size() << endl;
	tup->setFld(1, 1);
	tup->setFld(2, "adgggggg");
	tup->setFld(3, dec);
	tup->setFld(4, 56);
	tup->setFld(5, "ddddddddddddddddd"); 
	tup->setFld(6, dec); 
	tup->setFld(7, -90); 
	char* name2 = new char[10];
	float flt;
	std::cout << "field " << 1 << " is "<< tup->getFld(1, integer) << endl;
	std::cout << "field " << 2 << " is "<< tup->getFld(2, name2) << endl;
	std::cout << "field " << 3 << " is "<< tup->getFld(3, flt) << endl;
	std::cout << "field " << 4 << " is "<< tup->getFld(4, integer) << endl;
	std::cout << "field " << 5 << " is "<< tup->getFld(5, name2) << endl;
	std::cout << "field " << 6 << " is "<< tup->getFld(6, flt) << endl;
	std::cout << "field " << 7 << " is "<< tup->getFld(7, integer) << endl;
	std::cout << "that name was: " << name2 << endl;
	tup->print(types);
}
