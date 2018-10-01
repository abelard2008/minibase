/*
 ========================================================================
 Minibase 
 (c) Copyright R. Ramakrishnan, 
 University of Wisconsin at Madison.
 (1995-1996) All Rights Reserved.
 Version 0.0

 See 'COPYRIGHT' file for the full disclaimer and user agreement.
 ========================================================================
*/

#include <ostream>
#include <stdlib.h>
#include <string.h>
#include <strstream>

#include "ii_convertparsetree.h"
#include "ii_systemcatalogs.h"
#include "optimizer.h"
#include "typechk.h"
#include "joinmeth.h"
#include "nestloop.h"
#include "sortmerge.h"
#include "hashjoin.h"
#include "ioroutines.h"

#include <ext_sym_tabs.h>
#include <ext_token.h>
#include <parse_routines.h>
#include <globals.h>
#include <mr_syserr.h>

#include "ext_sys_defs.h"
#include "db.h"
#include "buf.h"
#include "hfpage.h"
#include "linearhashing.h"

#include "menu.h"
#include "catalog.h"
#include "planner.h"
#include "interpreter.h"

#include "component-tests.h"



int  MINIBASE_RESTART_FLAG = 0;


void table_init();
int OptimizeAndExecute(char *query_input_string);
void DoQuery();
void DoOpen();
void DoOutput();
void InitializeGlobals();
void MainMenu(int, char**);

char *input_string;

extern void yy_restart_lexical_analyzer ();

  /* This function closes the current database or bogus catalog, and leaves the
     process in its third possible state: i.e., with neither database nor
     catalog open. */
static void close_db()
{
    delete minibase_globals;
    minibase_globals = 0;
    delete systemCatalogs;
    systemCatalogs = 0;
}


void JoinMethodMenu()
{
  int i = 1;

  while ( std::cin && i != 0 )
  {
    std::cout << "Enter join method number to toggle.\nEnter 0 when done.:\n";
  
    std::cin >> i;
    if (i > 0)
      ToggleJoinMethod(i);
  }
}

void ListDatabases()
{
    std::cout << "end - databases.:" << std::endl;
}

void CreateDB()
{
    char buf[256] = {0};
    std::cout << "Enter the DB to create.:" << std::endl;
    std::cin >> std::ws;
    std::cin.get (buf, sizeof (buf), '\n');  
    std::cin.get();
    
    close_db();
    Status status;
    minibase_globals = new ExtendedSystemDefs( status, buf, 200 );

    if ( status == OK )
        std::cout << "Database " << buf << " created.:" << std::endl;
    else
      {
        close_db();
        std::cout << "Database " << buf << " NOT created.\n";
        minibase_errors.show_errors();
        std::cout << "\n.:\n";
      }
}

void OpenDB()
{
    char buf[256] = {0};
    std::cout << "Enter the DB to open.:" << std::endl;
    std::cin >> std::ws;
    std::cin.get (buf, sizeof (buf), '\n');  
    std::cin.get();
    
    close_db();
    Status status;
    minibase_globals = new ExtendedSystemDefs( status, buf );

    if ( status == OK )
        std::cout << "Database " << buf << " opened.:" << std::endl;
    else
      {
        close_db();
        std::cout << "Database " << buf << " NOT opened.\n";
        minibase_errors.show_errors();
        std::cout << "\n.:\n";
      }
}

void ComponentTests()
{
    //close_db();
    Menu menu;
    menu.runTests();
}
    

void UtilityLoop()
{
    if ( !minibase_globals )
      {
        std::cerr << "You must open a database first\n";
        return;
      }

    Menu menu;
    while ( menu.runMenu() == OK )
      {}
}

void ShowCat()
{
    if ( systemCatalogs )
        systemCatalogs->PrintFilename();
    else
        std::cout << "none.:\n";
}

void ListRelations()
{
    if ( !systemCatalogs )
      {
        std::cerr << "You must open a database or catalog first\n";
        return;
      }

    systemCatalogs->PrintRelations();
}

void ListAttributes()
{
    if ( !systemCatalogs )
      {
        std::cerr << "You must open a database or catalog first\n";
        return;
      }

    char buf[256] = {0};

    std::cout << "Enter the relation to get attributes of.:" << std::endl;
    std::cin >> std::ws;
    std::cin.get (buf, sizeof (buf), '\n');  
    std::cin.get();
    
    Plan_Relation* p = systemCatalogs->GetPlan_Relation(buf);
    if (p)
      {
        for(p->attributeList.GoHead(); 
            p->attributeList.CurrPtr();
            p->attributeList.GoNext())
            std::cout << p->attributeList.CurrPtr()->FullName() << ".:" << std::endl;
        delete p;
      } 
    std::cout << "end - attribute list.:" << std::endl;
}

void ListAccessMethods()
{
    if ( !systemCatalogs )
      {
        std::cerr << "You must open a database or catalog first\n";
        return;
      }

    char buf[256] = {0};
    
    std::cout << "Enter the relation to get access methods on.:" << std::endl;
    std::cin >> std::ws;
    std::cin.get (buf, sizeof (buf), '\n');  
    std::cin.get();
    
    Plan_Relation* p = systemCatalogs->GetPlan_Relation(buf);
    if (p)
      {
	for(p->accessMethodList.GoHead(); 
	    p->accessMethodList.CurrPtr();
	    p->accessMethodList.GoNext())
	    std::cout << p->accessMethodList.CurrPtr()->FullName() << ".:" << std::endl;
        delete p;
      }	
    std::cout << "end - accessmethod list.:" << std::endl;
}

#if 0
void DumpTuples()
{
    char buf[256] = {0};

    std::cout << "Enter the relation to dump.:" << std::endl;
    std::cin >> std::ws;
    std::cin.get (buf, sizeof (buf), '\n');  
    std::cin.get();

    std::cout << "1 - Dump to screen" << std::endl;
    std::cout << "2 - Dump to file" << std::endl;
    std::cout << "3 - Dump to printer" << std::endl;
    std::cout << "Where do you want to dump?.:" << std::endl;
    int i;    
    std::cin >> i;
    switch (i) 
    {
      case 1:
	std::cout << "Results for dumping " << buf << " should be here.:" << std::endl;
	break;
      case 2:
	// not finished
	break;
      case 3:
	break;
      default:
	break;
    }

}

void InsertTuple() 
{
    char buf[256] = {0};

    std::cout << "Enter the relation to insert a tuple into.:" << std::endl;
    std::cin >> std::ws;
    std::cin.get (buf, sizeof (buf), '\n');  
    std::cin.get();
    
    Plan_Relation* p = systemCatalogs->GetPlan_Relation(buf);
    if (p)
    {
	for(p->attributeList.GoHead(); 
	    p->attributeList.CurrPtr(); p->attributeList.GoNext())
	{
	    std::cout << p->attributeList.CurrPtr()->FullName() << ".:" << std::endl;
	    
	}
	std::cout << "end - attributes.:" << std::endl;
    }
}
#endif 

#define BUFSIZE 256

#if 0
void AlterRelation()
{
    char buf[BUFSIZE] = {0};
    
    std::cout << "Enter the relation to create or alter.:" << std::endl;
    std::cin >> std::ws;
    std::cin.get( buf, BUFSIZE, '\n');
    std::cin.get();
    
    Plan_Relation* p = systemCatalogs.GetPlan_Relation(buf);

    if (!p) 
    {
	p = new Plan_Relation(buf, systemCatalogs.database, 0, 0, 0);
	systemCatalogs.database->relations.InsertTail(p);
	std::cout << "Relation " << buf << " created.: " << std::endl;
    }
    else
    {
	std::cout << "Relation " << buf << " already present.: " << std::endl;
    }

    std::cout << p->originalName << ".:" << std::endl;
    std::cout << p->cardinality << ".:" << std::endl;
    std::cout << p->numPages << ".:" << std::endl;

    // tuplesize will be determined by the number of attributes.

    std::cout << "New name: [" << p->originalName << "] .:" ;
    std::cin.get( buf, BUFSIZE, '\n');
    std::cin.get();
    if (strcmp(buf,"") != 0)
    {
	p->name = buf;
	p->originalName = buf;
    }
    
    std::cout << "New cardinality: [" << p->cardinality << "] .:" << std::endl;
    std::cin.get( buf, BUFSIZE, '\n');
    std::cin.get();

    istrstream s1(buf);
    ulong ncard;	
    s1 >> ncard;
    if (s1.good())
	p->cardinality = ncard;
    
    std::cout << "New # pages: [" << p->numPages << "] .:" << std::endl;
    
    std::cin.get( buf, BUFSIZE, '\n');
    std::cin.get();

    istrstream s2(buf);
    ulong npages;	
    s2 >> npages;
    if (s2.good())
	p->numPages = npages;
}

void AlterAttribute()
{
    char buf[BUFSIZE] = {0};
    
    std::cout << "Enter the relation to alter an attribute on.:" << std::endl;
    std::cin >> std::ws;
    std::cin.get( buf, BUFSIZE, '\n');
    std::cin.get();
    
    Plan_Relation* p = systemCatalogs.GetPlan_Relation(buf);

    if (!p) 
    {
	std::cout << "Relation " << buf << " not present.: " << std::endl;
	return;
    }

    std::cout << "Enter the attribute to add or alter.:";
    
    std::cin >> std::ws;
    std::cin.get( buf, BUFSIZE, '\n');
    std::cin.get();

    Attribute *a = NULL;

    int nextOffset = 0;
    
    int id = 0;
    
    Attribute *ta;
    
    for(p->attributeList.GoHead();
	ta = p->attributeList.CurrPtr() ;
	p->attributeList.GoNext())
    {
        if ( ta->Name() != buf)
	    a = ta;
	
	id++;
	if ( ta->Size() +  ta->Offset() > nextOffset )
	    nextOffset = ta->Size() + ta->Offset();	
    }
    
    if (a ) // we found it!
    {
	std::cout << "Attribute " << buf << " already present.:" << std::endl;
    }
    else
    {
	a = new Attribute(buf, p, attrInteger, 4, nextOffset, id);
	a->minVal = 0;
	a->maxVal = 999999999;	
    }

    std::cout << a->name << ".:" << std::endl;
    std::cout << a->type << ".:" << std::endl;
    std::cout << a->minVal << ".:" << std::endl;
    std::cout << a->maxVal << ".:" << std::endl;
    std::cout << a->size << ".:" << std::endl;
    std::cout << a->offset << ".:" << std::endl;
    
    // tuplesize will be determined by the number of attributes.

    std::cout << "New name: [" << a->name << "] .:" ;
    std::cin.get( buf, BUFSIZE, '\n');
    std::cin.get();
    if (strcmp(buf,"") != 0)
    {
	a->name = buf;
    }
    
    
    std::cout << "New type: [" << a->type << "] .:" ;
    
    std::cin.get( buf, BUFSIZE, '\n');
    std::cin.get();

    int i;
    
    for ( i =0; buf[i] != ' ' && buf[i] != '\n' && buf[i] != '\0'; i++)
	;	// do nothing

    if (buf[i] != '\0')
    {
	switch (buf[i]) 
	{
	  case 'R':
	  case 'r':	    
	    a->type = attrReal; break;
	  case 'I':
	  case 'i':
	    a->type = attrInteger;
	    break;
	  case 's':
	  case 'S':
	    a->type = attrString;
	    break;
	  default:
	    break;
	}
    }
    
		
    std::cout << "New minVal: [" << a->minVal << "] .:" ;
    std::cin.get( buf, BUFSIZE, '\n');
    std::cin.get();
    istrstream s2(buf);
    AttrValue newMinVal;
    newMinVal.Read(s2, a->type);
    if (s2.good())
	a->minVal = newMinVal;

    std::cout << "New maxVal: [" << a->maxVal << "] .:" ;
    std::cin.get( buf, BUFSIZE, '\n');
    std::cin.get();
    istrstream s3(buf);
    AttrValue newMaxVal;
    newMaxVal.Read(s3, a->type);
    if (s3.good())
	a->maxVal = newMaxVal;
    
    std::cout << "New size: [" << a->size << "] .:" ;
    std::cin.get( buf, BUFSIZE, '\n');
    std::cin.get();
    istrstream s4(buf);
    int newSize;
    s4 >> newSize;
    if (s4.good())
	a->size = newSize;
    
    std::cout << "New offset: [" << a->offset << "] .:" ;
    std::cin.get( buf, BUFSIZE, '\n');
    std::cin.get();
    istrstream s5(buf);
    int newOffset;
    s5 >> newOffset;
    if (s5.good())
	a->offset = newOffset;
    
    nextOffset = 0;
    for(p->attributeList.GoHead();
	ta = p->attributeList.CurrPtr() ;
	p->attributeList.GoNext())
    {
	if ( ta->Size() +  ta->Offset() > nextOffset )
	    nextOffset = ta->Size() + ta->Offset();	
    }
    p->tupleSize = nextOffset;		
}
#endif 0

void
PrintUsage(){

  fprintf(stderr,"Usage::\n");
  fprintf(stderr,"minibase  \n");
  fprintf(stderr,"-d < (string) database name >\n");
  fprintf(stderr,"-s < (int) size of the db in pages>\n");
  fprintf(stderr,"-b < (int) size of the buffer pool in pages>\n");
  fprintf(stderr,"-r (Restart Mode) \n");
  fflush(stderr);
  return;
}

////////////////////////////////////////////////////////////////////////
// * Function Name : ProcessCommandLineArgs
//   -d <string> : database name
//   -s <int> : size of the db in pages
//   -b <int> : size of the buffer pool in pages
//   -l <int> : size of the log in pages
//

void
ProcessCommandLineArgs(int argc,char** argv,int* dbsize, int* bufpoolsize,
		char* dbname){

 int i;

 int dbsize_set = 0; 

 if( argc < 2) {
	PrintUsage();
	exit(1);
 }
	
 for (i = 1; i < argc; i++){
     char *flag = argv[i];
	if (flag[0] != '-') {
	     std::cerr << "Bad command line flag "<< argv[i] << std::endl;
	     PrintUsage();
	     exit(1);
	}
    switch (flag[1]) {
       case 'd' :
	   if( (++i) < argc) 
	     strcpy(dbname,argv[i]);
	   break;
       case 's' :
	   if( (++i) < argc) {
	     *(dbsize) = atoi(argv[i]);
	     dbsize_set = 1;
	   }
	   break;
       case 'b' :
	   if( (++i) < argc)
	     *(bufpoolsize) = atoi(argv[i]);
	   break;
       case 'r' :
	      *(dbsize) = 0;
	      dbsize_set = 1;
	      MINIBASE_RESTART_FLAG = 1;
	   break;
       default :
	  PrintUsage();
	       exit(1);
    }
  }

  if(!dbsize_set) *(dbsize) = 0; // open in the restart mode.

  fprintf(stderr,"Returning %s dbname, %d size and %d bufrepl\n",
	  dbname, *(dbsize),*(bufpoolsize));
  fflush(stderr);

  return;
}

void InitializeGlobals(int argc, char** argv){
 
Status status;
int dbsize,bufpoolsize;
char* dbname;


    dbname = new char[50]; 

    ProcessCommandLineArgs(argc,argv,&dbsize,&bufpoolsize,dbname);
								

    minibase_globals = new ExtendedSystemDefs(status, dbname, dbsize,
                       bufpoolsize, "Clock");
 
    /*
    status = MINIBASE_RECMGR->Restart();

    if(status != OK) {
          printf("Unable to restart! Exiting ...\n");
	  delete minibase_globals;
	  exit(1);
    }

    status = MINIBASE_XACTMGR->commit();
    if(status != OK) {
          printf("Unable to Commit transaction ...\n");
	  delete minibase_globals;
	  exit(1);
    }

    status = MINIBASE_RECMGR->Checkpoint();
    if(status != OK) {
          printf("Unable to Checkpoint - 1 ! Exiting ...\n");
	  delete minibase_globals;
	  exit(1);
    }
    status = MINIBASE_RECMGR->Checkpoint();
    
    if(status != OK) {
          printf("Unable to Checkpoint - 2 ! Exiting ...\n");
	  delete minibase_globals;
	  exit(1);
    }
    */

    delete dbname;
    return ;
}


void MainMenu(int argc, char** argv)
{

    InitializeGlobals(argc,argv);

    int i = -1;
    while ( std::cin.good() && i != 0 )
      {

        std::cout <<
            "[1]  -  Enter SQL\n"
            "[2]  -  Open Optimizer Catalog\n"
            "[3]  -  Toggle Active Join Methods\n"
            "[4]  -  List JoinMethods\n"
            "[5]  -  List Relations\n"
            "[6]  -  List Access Methods for a Relation\n"
            "[7]  -  Flip Index Only Plans\n"
            "[8]  -  Output Current Catalog\n"
            "[9]  -  Show what catalog/database is open\n"
            "[10] -  Minibase UTILITIES\n"
            "[11] -  List Attributes for a Relation\n"
            
            "[0]  -  Exit.:\n";

        i = -1;
        std::cin >> i;

        switch (i)
          {
            case 1:  

		     DoQuery();
		     break;
            case 2:  
		     DoOpen();
		     break;
            case 3:  
		     JoinMethodMenu();
		     break;
            case 4:  
		     ShowJoinMethods(TRUE); 
		     break;
            case 5: 
		     ListRelations();   
		     break; 
            case 6:  
		     ListAccessMethods(); 
		     break;
            case 7:  
		     Optimizer_IndexOnly = ! Optimizer_IndexOnly; 
		     break;
            case 8:  
		     DoOutput(); 
		     break;
            case 9:  
		     ShowCat();         
		     break;
            case 10: 
		     UtilityLoop();         
		     break;
            default: 
		     delete minibase_globals;
		     minibase_globals = NULL;
		     break;
          }
      } //end of while loop
}


/*----------------- old stuff

//           case 5:  CreateDB();        break;
//            case 6:  ListDatabases(); break;
//           case 7:  OpenDB();          break;
            case 8:  RunTests();        break;
            case 5: ListRelations();   break;
            case 11: ListAccessMethods(); break;
//            case 12: DumpTuples();      break;
//            case 13: InsertTuple();     break;
            case 14: Optimizer_IndexOnly = ! Optimizer_IndexOnly; break;
            case 15: DoOutput();        break;
//            case 16: AlterRelation();   break;
//            case 17: AlterAttribute();  break;
            case 18: UtilityLoop();     break;
            case 19: ShowCat();         break;
	 
            default: break;
------------------------*/

void DisplayMenu(int argc,char** argv){

    std::cout << "\n"
        "***************************\n"
        "\n"
        "Welcome to Minibase.\n"
        "\n"
        "***************************\n"
        "\n";

    int i = 0;
    while ( std::cin.good() && i != 3 ){
        std::cout <<
            "1  -  Minibase\n"
            "2  -  Component Tests\n"
            "3  -  Exit.:\n";
        i = -1;
        std::cin >> i;
        switch (i)
          {
            case 1:  MainMenu(argc,argv);         break;
            case 2:  ComponentTests();          break; 
	    default: break;
	  }
    }

    std::cout << "\n"
        "***************************\n"
        "\n"
        "Thank you for using Minibase.\n"
        "\n"
        "***************************\n"
        "\n";
}

void DoOpen()
{
  char buf[256] = {0};    

  std::cout << "Enter catalog filename.:\n";
  std::cout.flush();
  std::cin >> std::ws;
  std::cin.get (buf, sizeof (buf), '\n');  
  std::cin.get();

  close_db();
  systemCatalogs = new ii_BogusSystemCatalogs(buf); 
}

void DoOutput()
{
  char buf[256] = {0};    

  std::cout << "Enter filename to write current catalog to.:\n";
  std::cout.flush();
  std::cin >> std::ws;
  std::cin.get (buf, sizeof (buf), '\n');  
  std::cin.get();
 
  systemCatalogs->OutputCatalog(buf); 
}



void PruningMenu(Optimizer & o)
{
  int i = 1;
  while (i != 0)
  {
    std::cout << "Enter level to toggle pruning for.\nEnter 0 when done.:\n";
  
    std::cin >> i;
    o.TogglePruning(i);
  }
  
}


void AccessMethodMenu(Optimizer & o)
{
  int i = 1;
  while (i != 0)
  {
    std::cout << "Enter Access Method ID to toggle.\nEnter 0 when done.:\n";
  
    std::cin >> i;
    o.ToggleOmits(i);
  }
  
}



int Execute(PlanNode* bestPlan)
{
    if (!bestPlan)
      {
        std::cerr << "No best plan was selected.  Bailing out..:\n";
        return 0;
      }

//    bestPlan->ShowRecursive();      // for debugging only

      // execution variables
    AttributeList attrs;
    Status status;
    Iterator* queryIter = bestPlan->interpret(attrs, status);
    if (status != OK)
      {
        minibase_errors.show_errors();
        std::cout << "\n.:\n";
        return 0;
      }

    Converter c(bestPlan, attrs);
    AttrType types[MAX_ATTR_LIST];
    short strLens[MAX_ATTR_LIST];
    c.convert(types, strLens);
    Tuple* tuple;

    while ( (status = queryIter->get_next(tuple)) == OK )
        tuple->print(types);

    delete queryIter;

    if (status != DONE)
      {
        minibase_errors.show_errors();
        std::cout << "\n.:\n";
        return 0;
      }
    
    std::cout << std::endl << std::endl << ".:" << std::endl;
    return 1;
}



void OptimizerMenu(Optimizer & o)
{

    int i = 0;

    while (i != 9)
      {
        std::cout << "1  -  Execute\n";
        std::cout << "2  -  Pruning\n";
        std::cout << "3  -  Toggle Active Join Methods\n";
        std::cout << "4  -  List JoinMethods\n";
        std::cout << "5  -  Toggle Active Access Methods\n";
        std::cout << "6  -  List Databases\n";
        std::cout << "8  -  Recompute Plans\n\n";
        std::cout << "10 -  List Relations\n";
        std::cout << "11 -  List Access Methods for a Relation\n";   
//        std::cout << "12 -  Dump Tuples for a Relation\n";
        std::cout << "14 -  Flip Index Only Plans\n";
        std::cout << "15 -  Output Current Catalog\n";    

        std::cout << "9  -  Close Menu.:\n";

        std::cin >> i;
        switch (i)
          {
            case 0: std::cout << "1\n"; break;
            case 1: Execute(o.GetBestSolution()); break;	 
            case 2: PruningMenu(o); break;
            case 3: JoinMethodMenu(); break;
            case 4: ShowJoinMethods(); break;
            case 5: AccessMethodMenu(o); break;
            case 6: ListDatabases(); break;
            case 8: o.Optimize(); o.ShowSolutions(); break;
            case 10: ListRelations(); break;
            case 11: ListAccessMethods(); break;
//            case 12: DumpTuples(); break; 
            case 14: Optimizer_IndexOnly = ! Optimizer_IndexOnly; break;
            case 15: DoOutput(); break;

            default: break;
          }
      }

}


void DoQuery()
{
    if ( !systemCatalogs )
      {
        std::cerr << "You must open a database or catalog first\n";
        return;
      }

  char buf[8192] = {0};

  std::cout << "Query (end with a `;').:\n";
  std::cout.flush();
  std::cin.get (buf, sizeof (buf), ';');
  std::cin.get(); // flush the ';'
  std::cin.get(); // flush the '\n'
  
  OptimizeAndExecute (buf);

}


int OptimizeAndExecute(char *query_input_string)
{
#if 0  
    ParserErr.ClearState();
    //    yy_restart_lexical_analyzer ();
      
    input_string = query_input_string;

    table_init ();

    //if (yyparse())
    //   ParserErr.SetState(SysErr::PParseError);
      // the parse tree will be returned in a global variable Entry

    if ( ParserErr.NoError() )
        pp_typechk (Entry, false);

    if ( ParserErr.NoError() )
      {
        ii_ConvertFromParseTree cfpt(Entry);

        if ( ParserErr.NoError() )
          {
            std::cout << "no errors.:\n";
            Optimizer o(cfpt.GetState());
            o.Optimize();
            o.ShowSolutions();
            OptimizerMenu(o);
          }
      }
    if ( !ParserErr.NoError() )
      {
        std::cerr << "error - ";
        ParserErr.ShowErr();
        std::cerr << ".:\n";
      }

    freenode( Entry );
#endif    
    return 0;
}

void AddModelToList(JoinMethod* Model)
{
    masterJoinMethodList.Insert(Model);
    activeJoinMethodList.Insert(Model);
}

int main(int argc, char** argv)
{

    JoinMethod* joins[5];

    AddModelToList(joins[0] = new NestedLoopsRightSubtreeInner);
    AddModelToList(joins[1] = new PageNestedLoops);
    AddModelToList(joins[2] = new IndexNestedLoops);
    AddModelToList(joins[3] = new SortMerge);
    AddModelToList(joins[4] = new HashJoin);
    
    //    std::cerr = std::cout;

    DisplayMenu(argc,argv);
    for ( int i = 0; i < 5; i++)
	delete joins[i];
    
    close_db();
    return 0;
}
