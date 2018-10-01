#include "menu.h"
#include "minirel.h"
#include "ext_sys_defs.h"
#include "maincatalog.h"
#include "utility.h"
#include <fstream>
#include <stdio.h>

#include "component-tests.h"
#include "planner.h"
#include "sortmerge.h"
#include "nestloop.h"


extern SystemDefs* minibase_globals;

Menu::Menu()
{
}

Menu::~Menu()
{
}


Status Menu::runMenu()
{
    int choice = -1;

    while ( std::cin.good() && choice != 0 )
      {
        std::cout << "\nEnter a choice\n"
            "[1]  - create a relation\n"
            "[2]  - build an index\n"
            "[3]  - drop an index\n"
            "[4]  - delete a relation\n"
            "[5]  - add a tuple\n"
            "[6]  - delete a tuple\n"
            "[7]  - view a relation\n"
            "[8]  - list all indexes on a relation\n"
            "[9] -  run stats\n"

//          "[11] - load from a text file\n"
//          "[12] - destroy a database\n"


            "[0]  - quit.:\n";

        choice = -1;
        std::cin >> choice;

        switch (choice)
          {
            case 0: return DONE;
            case 1: 
		    return Menu::createRel();
            case 2: 
		    return Menu::buildIndex(); 
            case 3: 
		    return Menu::dropIndex();
            case 4: 
		    return Menu::deleteRel();
            case 5: 
		    return Menu::addTuple();
            case 6: 
		    return Menu::deleteTuple();
            case 7: 
		    return Menu::viewRel();
            case 8: 
		    return Menu::viewIndices();
            case 9: 
		    return Menu::runStats();

//          case 11: return Menu::load();
//          case 12: return Menu::destroyDB();

            default: break;

#if 0   // Keep some old cases for posterity
            case 8:
              {
                int number;
                char** names;
                Status s = Menu::getDBs(number, names);
                for (int i = 0; i < number; i++)
                  {
                    std::cout << names[i] << std::endl;
                    delete [] names[i];
                  }
                delete [] names;
                return s;
              }
            case 13:
              {
                int n;
                char** names1;
                Status st;
                st = Menu::getRelns(n, names1);
                std::cout << "Relations for database " << MINIBASE_DB->db_name() << ':' << std::endl;
                for (int j = 0; j < n; j++)
                  {
                    std::cout << "Relation " << j << " is " << names1[j] << std::endl;
                    delete [] names1[j];
                  }
                delete [] names1;
                break;
              }
#endif
        }  // End of switch
      }

    return DONE;
}



Status Menu::createRel()
{

      Status status;
      int number = 0;
      std::cout << "Create a Relation:\n";
      
      char* name = new char[21];
      std::cout << "Enter the name of the relation (20 chars or less): ";
      std::cin >> std::ws;
      std::cin.get(name, 20);
      std::cin.ignore(20, '\n');

      while ((number < 1) || (number > 20))
	{
	  std::cout << "How many attributes: ";
	  std::cin >> number;
	  std::cin.ignore(10, '\n');
	}
      
      attrInfo* attrList;
      attrList = new attrInfo[number];
      
      for (int i = 0; i < number; i++)
	{
	  std::cout << "Enter attribute name (30 characters or less): ";
	  std::cin.get(attrList[i].attrName, 30);
	  std::cin.ignore(30, '\n');

	  char type = 'z';
	  while (!((type == 's') || (type == 'i') || (type == 'f')))
	    {
	      std::cout << "Enter attribute type (s, i, f): ";
	      std::cin >> type; 
	      std::cin.ignore(10, '\n');
	    }

	  switch (type)
	    {
	    case 's':
	      attrList[i].attrType = attrString;
	      std::cout << "Enter the number of characters are allowed: ";
	      std::cin >> attrList[i].attrLen; 
	      std::cin.ignore(20, '\n');
	      break;
	    case 'i':
	      attrList[i].attrType = attrInteger;
	      attrList[i].attrLen = sizeof(int);
	      break;
	    case 'f':
	      attrList[i].attrType = attrReal;
	      attrList[i].attrLen = sizeof(float);
	      break;
	    default:
	      std::cerr << "Something is wrong!!!\n";
	      return CATALOG;
	    }
	} 
      status = MINIBASE_CATALOGPTR->createRel(name, number, attrList) ;

      if(status == OK)
	 std::cout << std::endl << "Relation " << name<< " successfully created." << std::endl;
      else
	 std::cout << std::endl << "Failure to create relation " << name << std::endl;

      delete [] attrList;
      delete name;
      return status;
}  // End Menu::createRel


Status Menu::buildIndex()
{
      std::cout << "Add an index\n";
      char* name = new char[21];
      char* name2 = new char[21];
      Status status;
      
      std::cout << "On what relation: ";
      std::cin >> std::ws;
      std::cin.get(name, 20);
      std::cin.ignore(20, '\n');
      
      std::cout << "On what attribute: ";
      std::cin.get(name2, 20);
      std::cin.ignore(20, '\n');
      
      
      char index = 'z';
      
      while ((index != 'h') && (index != 'b'))
	{ 
	  std::cout << "Pick an index [h]ash or [b]tree - ";
	  std::cin >> index;
	}
      
      if (index == 'h')
	status = MINIBASE_CATALOGPTR->addIndex(name, name2, Hash, 1);
      else
	status = MINIBASE_CATALOGPTR->addIndex(name, name2, B_Index, 1);
      
      if (status != OK)
	{
	  std::cerr << "Failure to add index.";
	  delete name;
	  delete name2;
	  return CATALOG;
	}
     
      std::cout << "Index added successfully\n\n";
      delete name;
      delete name2;
      return OK;
}


Status Menu::dropIndex()
{
     Status status; 
      std::cout << "Drop an index\n";
      char* name = new char[21];
      char* name2 = new char[21];
      
      std::cout << "On what relation: ";
      std::cin >> std::ws;
      std::cin.get(name, 20);
      std::cin.ignore(20, '\n');
      
      std::cout << "On what attribute: ";
      std::cin.get(name2, 20);
      std::cin.ignore(20, '\n');
      
      
      char index = 'z';
      
      while ((index != 'h') && (index != 'b'))
	{ 
	  std::cout << "Pick an index [h]ash or [b]tree - ";
	  std::cin >> index;
	}
      
      if (index == 'h')
	status = MINIBASE_CATALOGPTR->dropIndex(name, name2, Hash);
      else
	status = MINIBASE_CATALOGPTR->dropIndex(name, name2, B_Index);
      
      if (status != OK)
	{
	  std::cerr << "Failure to drop index.";
	  delete name;
	  delete name2;
	  return CATALOG;
	}
      
      delete name;
      delete name2;
      return OK;
}  // End of Menu::dropIndex


Status Menu::dumpCatalog()
{
      std::cout << "Dump Catalog: \n";
      if (MINIBASE_CATALOGPTR->dumpCatalog("temporary/bogus.catalog.data") == OK)
	std::cout << "Catalog is in temporary/bogus.catalog.data";
      else
	std::cout << "Failure to dump Catalog";
      return OK;
}  // End of Menu::dumpCatalog



Status Menu::deleteRel()
{


      char* name = new char[21];

      std::cout << "Name of the relation that you want to delete: ";
      std::cin >> std::ws;
      std::cin.get(name, 21);
      std::cin.ignore(21, '\n');
      
      if (MINIBASE_CATALOGPTR->destroyRel(name) == OK)
	std::cout << "relation: " << name << " successfully destroyed\n";
      else
	std::cout << "Failure to destroy relation: " << name << std::endl;

      return OK;

} // End of Menu::deleteRel()



Status Menu::addTuple()
{
      // insertRecordUT(char * relation, int attrCnt, attrNode attrList[])

      char* name = new char[21];
      int number = 0;
      int i;

      std::cout << "Enter the relation where tuple should go: ";
      std::cin >> std::ws;
      std::cin.get(name, 21);
      std::cin.ignore(21, '\n');
      
      Status status;
      AttrDesc *attrs;
      status = MINIBASE_CATALOGPTR->getRelAttributes(name, number, attrs);

      attrNode* tuple = new attrNode[number];

      for (i = 0; i < number; i++)
	{
	  tuple[i].attrName = attrs[i].attrName;
	  std::cout << "Enter a value for attribute " << tuple[i].attrName << ": ";
	  tuple[i].attrValue = new char[20];
	  std::cin.get(tuple[i].attrValue, 20);
	  std::cin.ignore(20, '\n');
	  std::cout << "You entered \t\t" << tuple[i].attrValue << std::endl;

	  // Need to check for min / max values
	}

      if (insertRecordUT(name, number, tuple) == OK)
	std::cout << "tuple inserted successfully into " << name << std::endl;
      else
	std::cout << "Failure to insert tuple\n";


      for (i = 0; i < number; i++)
	delete tuple[i].attrValue;
      delete [] tuple;
      delete name;

      delete [] attrs;
      return OK;
} // End of Menu::addTuple


Status Menu::deleteTuple()
{

      char* name = new char[21];
      int number = 0;
      int i;
      
      std::cout << "Enter the relation from which you would ";
      std::cout << "like to delete a tuple: ";
      std::cin >> std::ws;
      std::cin.get(name, 21);
      std::cin.ignore(21, '\n');

      AttrDesc *attrs;

      Status status;
      status = MINIBASE_CATALOGPTR->getRelAttributes(name, number, attrs);

      if (status != OK)
      {
        std::cout << "Unknown relation " << name << std::endl;
        delete name;
        return CATALOG;
      }

      int choice;
      attrNode* tuple;
      tuple = new attrNode[number];

      std::cout << "Attributes of relation " << name << ":\n";
      for (i = 0; i < number; i++)
	{
	  std::cout << '\t' << i << "  " << attrs[i].attrName << '\t';
	  if (attrs[i].attrType == attrString)
	    std::cout << "of type <string>" << std::endl;
	  else
	    if (attrs[i].attrType == attrReal)
	      std::cout << "of type <real>" << std::endl;
	    else
	      std::cout << "of type <integer>" << std::endl;
	}

     
      std::cout << "Select the number of the desired attribute: ";
      std::cin >> choice;
      std::cin.ignore(21, '\n');

//      tuple[choice].attrName = new char[20];
//      memcpy(tuple[choice].attrName, attrs[choice].attrName, 20);
     tuple[choice].attrName = attrs[choice].attrName;
     tuple[choice].attrValue = new char[20];
      std::cout << "Enter the value for " << tuple[choice].attrName << ": ";
      std::cin.get(tuple[choice].attrValue, 21);
      std::cin.ignore(21, '\n');
      
      if (deleteRecordUT(name, tuple[choice]) == OK)
	std::cout << "Record was deleted successfully\n";
      else
	std::cout << "Failure to delete record\n";
      
      delete tuple[choice].attrValue;
      delete [] tuple;
      delete [] attrs;
      delete name;

      return OK;
} // End of Menu::deleteTuple
     


Status Menu::viewRel()
{
      char* name = new char[21];
      
      std::cout << "Enter the relation to view: ";
      std::cin >> std::ws;
      std::cin.get(name, 21);
      std::cin.ignore(21, '\n');

      RelDesc record;
      Status status = MINIBASE_CATALOGPTR->getRelationInfo(name, record);
      if (status != OK)
      {
        std::cout << "Failure to open " << name << std::endl;
        delete name;
        return CATALOG;
      }

      
      HeapFile* hf = new HeapFile(name, status);
      if (status != OK)
      {
	std::cout << "Failure to open " << name << std::endl;
	delete name;
	return CATALOG;
      }
      
      AttrType* types;
      int arraysize;
      short* sizes;
      RID rid;
      
      Tuple* tup = (Tuple *) new char[Tuple::max_size];
      status = MINIBASE_ATTRCAT->getTupleStructure(name, arraysize, types, sizes);

      if (status != OK)
      {
	std::cout << "Failure to get tuple structure" << name << std::endl;
	delete name; delete hf; delete tup;
	return CATALOG;
      }

      tup->setHdr(arraysize,types,sizes);

      Scan* scan;
      int length = tup->size();
      scan = hf->openScan(status);
      while((scan->getNext(rid, (char*)tup, length)) != DONE)
	tup->print(types);  // AttrType
      
      delete scan;
      delete hf;
      delete tup;
      delete types;
      delete name;
      delete [] sizes;

      return OK;
}  // End of viewRel

//Beginning of Additions for Transaction manipulation


Status Menu::viewIndices()
{
      char* name = new char[21];
      
      std::cout << "\nEnter the relation to view its indicies: ";
      std::cin >> std::ws;
      std::cin.get(name, 21);
      std::cin.ignore(21, '\n');
      
      int number, i;
      IndexDesc* indexes;

      MINIBASE_CATALOGPTR->getRelIndexes(name, number, indexes);
      
      for(i = 0; i < number; i++)
	{
	  if (indexes[i].accessType == Hash)
	    std::cout << "Hash index on " << indexes[i].attrName << std::endl;
	  else
	    std::cout << "Btree index on " << indexes[i].attrName << std::endl;
	}

      delete name;
      delete [] indexes;

      return OK;
} // End of Menu::viewIndices


Status Menu::runTests()
{

  int choice;

  std::cout << "\nTest Choices:\n";
  std::cout << "[1] - Joins\n";
  std::cout << "[2] - linear hashing access method\n";
  std::cout << "[3] - disk space management\n";
  std::cout << "[4] - buffer management\n";
  std::cout << "[5] - heap file storage\n";
  std::cout << "[6] - Btree index\n";
  std::cout << "[9] - quit\n";
  std::cout << "Pick one: ";
  std::cin >> choice;

  switch (choice)
  {
    case 1:
      {
        std::cout << "\nJoins Tests were selected.\n";
        JoinsTests* tester1 = new JoinsTests;

        tester1->runTests();
        std::cout << "\nTests are over.\n";
        delete tester1;
        std::cout <<"Joins Tests have not been debugged for shared memory"<< std::endl;
        break;
      }

    case 2:
      {
        std::cout << "\nLinear Hash Tests were selected.\n";
        LinearHashTests* tester2 = new LinearHashTests;

        tester2->runTests();
        std::cout << "\nTests are over.\n";
        delete tester2;
        break;
      }

    case 3:
      {
        DBDriver tester3;
        tester3.runTests();
        break;
      }

    case 4:
      {
        BMTester tester4;
        tester4.runTests();
        break;
      }

    case 5:
      {
        HeapDriver tester5;
        tester5.runTests();
        break;
      }

    case 6:
      std::cout << "\nBTree Tests were selected.\n";
      
      BTreeTest* tester6;
      tester6 = new BTreeTest;
      tester6->runTests();
      std::cout << "\nTests are over.\n";
      delete tester6;
    
      break;


    case 9:
      std::cout << "\nQuitting tests.\n";
      return OK;

    default:
      return CATALOG;
  }

  return OK;
}



Status Menu::load()
{
  std::cout << "THIS DOESN'T WORK YET!!!!\n";
  char* name = new char[20]; 
  std::cout << "Enter the relation into which you want to insert records: ";
  std::cin >> std::ws;
  std::cin.get(name, 20);
  std::cin.ignore(100, '\n');

  char* name2 = new char[20];
  std::cout << "Enter the filename which contains the records: ";
  std::cin.get(name2, 20);
  std::cin.ignore(100, '\n');

  Status status;
  status = loadUT(name, name2); 

  delete name2;
  delete name;
  if (status == OK)
  {
    std::cout << "Load was successful.\n";
    return OK;
  }
  else
    std::cout << "Load failed.\n";

  return CATALOG;
}

Status Menu::getDBs(int& number, char**& names){
FILE* fp;
char  num[10];

	fp = fopen("minibase-databases","r");

	if( fp == NULL) {
		std::cout << "Failure to open minibase-databases for reading"<< std::endl;
		return CATALOG;
	}

	// read the contests of minibase-databases 
	if(!feof(fp)) {
	   fgets( num,10,fp);
           number = atoi(num); // Number of databases already created
           names  = new char*[number];
           for (int i = 0; i < number; i++)
                 names[i] = new char[MINIBASE_MAXARRSIZE];
           int i = 0;
           while(!feof(fp)) { //sit in a loop and get all DB names.
	         fgets(names[i], MINIBASE_MAXARRSIZE, fp);
	   }
	   fclose(fp);
	   return OK;
	   
	}else { // the file is empty at startup 
	   number = 0;
	   fclose(fp);
	   return OK;
	}

	fclose(fp);
	return CATALOG;  //error - should never get here.
}

Status Menu::destroyDB()
{
  std::cout << "This operation can be performed on the current\n";
  std::cout << "database or on another database.\n";

  char* choice;
  choice = new char[50];

  std::cout << "Enter name: ";
  std::cin >> std::ws;
  std::cin.getline(choice, 50);

  int number;
  char** names;

  Menu::getDBs(number, names);


  int found = 0, i;
  for ( i = 0; i < number; i++)
  {
    if (strcmp(names[i], choice) == 0)    // match!!
      found = 1;
  }

  if (!found)
  {
    std::cout << "Sorry, but I couldn't locate the database " << choice << std::endl;
    for (int j = 0; j < number; j++)
      delete [] names[j];
    delete names;
    return OK;
  }

  std::ofstream db_file("minibase-databases");
  if (!db_file)
  {
    std::cout << "Unable to destroy  :(\n";
    return CATALOG;
  }

  db_file << (number - 1);
  db_file << std::endl;
  for ( i = 0; i < number; i++)
    if (strcmp(names[i], choice) != 0)    // no match
    {
      db_file << names[i]; 
      db_file << std::endl;
    }

  std::cout << "Database " << choice << " located.  Deleting.\n";
  std::cout << "You should now type \"rm " << choice << "\" at the command line to delete\n";
  std::cout << "the database\n";
  for ( i = 0; i < number; i++) 
    delete [] names[i];
  delete names;
  delete choice;
  return OK;
}


// Amit BEGIN

Status Menu::runStats()
{
   std::cout << "Generating stats...";
   if (MINIBASE_CATALOGPTR->runStats("temporary/bogus.catalog.data") == OK)
      std::cout << "Catalog is in temporary/bogus.catalog.data";
   else
      std::cout << "Failure to dump Catalog";

   std::cout << std::endl;
   return OK;
}  // End of Menu::runStats

// Amit END


int Menu::existsDB(const char* name)
{
    int num;
    char** names;

    Menu::getDBs(num, names);

    for (int index = 0; index < num; index++)
        if (strcmp(names[index], name) == 0)
          {
            for (int j = 0; j < num; j++)
                delete [] names[j];
            delete [] names;
            return 1;
          }
    for (int j = 0; j < num; j++)
        delete [] names[j];
    delete [] names;
    return 0;
}


Status Menu::getRelns(int& number, char**& names)
{

  HeapFile* hf;
  Status status;
  RID rid;
  int length;

  Tuple* tuple;
  tuple = (Tuple*) new char [Tuple::max_size];

  hf = new HeapFile(RELCATNAME, status);
  if (status != OK)
  {
    std::cout << "Error opening relation " << RELCATNAME;
    std::cout <<  " in Menu::getRelns" << std::endl;
  }
  
  number = hf->getRecCnt();
  names = new char*[number];
  

  Scan* scan = hf->openScan(status);

  int i = 0;
  while (scan->getNext(rid, (char*)tuple, length) != DONE)
  {
    char* relname;
    tuple->getFld(1, relname);
    names[i] = new char[strlen(relname) + 1];
    strcpy(names[i], relname);
    i++;
  }
  delete [] tuple;
  delete hf;
  delete scan;
  return OK;
}


void Menu::initialize_joins()
{
JoinMethod* SortMergeModel = new SortMerge;
JoinMethod* NestLoopModel = new NestedLoopsRightSubtreeInner;
JoinMethod* IndexNestedLoopsModel = new IndexNestedLoops;
JoinMethod* PageNestedLoopsModel = new PageNestedLoops;
masterJoinMethodList.Insert(NestLoopModel);
activeJoinMethodList.Insert(NestLoopModel);
masterJoinMethodList.Insert(PageNestedLoopsModel);
activeJoinMethodList.Insert(PageNestedLoopsModel);
masterJoinMethodList.Insert(IndexNestedLoopsModel);
activeJoinMethodList.Insert(IndexNestedLoopsModel);
masterJoinMethodList.Insert(SortMergeModel);
activeJoinMethodList.Insert(SortMergeModel);
}

