#ifndef _MENU_H_
#define _MENU_H_

#include "minirel.h"

class Menu {

public:
Menu();
~Menu();

Status runMenu();
Status createRel();
Status buildIndex();
Status dropIndex();
Status dumpCatalog();
Status deleteRel();
Status addTuple();
Status deleteTuple();
Status viewRel();
Status viewIndices();
Status runTests();
Status load();
Status getDBs(int& number, char**& names);
Status destroyDB();
Status runStats();					// Amit
int existsDB(const char*);
Status getRelns(int& number, char**& names);
void initialize_joins();


};

#endif

