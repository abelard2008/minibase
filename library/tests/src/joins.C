#include <ctype.h>

#include "test.h"
#include "minirel.h"
#include "tuple.h"
#include "iterator.h"

#include "joins-example.h"
#include "joins_tests.h"

#include "joins.h"
#include "sort.h"
#include "dupl_elim.h"

#include "file_scan.h"

struct S {
   int   sid;
   char  sname[25];
   int   rating;
   float age;
}

Sailors[] =
{
    {10, "Mike Carey",         9, 40.3},
    {21, "David Dewitt",      10, 47.2},
    {29, "Tom Reps",           7, 39.1},
    {31, "Jeff Naughton",      5, 35.0},
    {35, "Miron Livny",        7, 37.6},
    {37, "Marv Solomon",      10, 48.9},
    {39, "Anne Condon",        3, 30.3},
    {47, "Charles Fischer",    6, 46.3},
    {49, "James Goodman",      4, 50.3},
    {50, "Mark Hill",          5, 35.2},
    {53, "Bob Holloway",       9, 53.6},
    {54, "Susan Horowitz",     1, 34.2},
    {57, "Yannis Ioannidis",   8, 40.2},
    {59, "Deborah Joseph",    10, 39.8},
    {61, "Landwebber",         8, 56.7},
    {63, "James Larus",        9, 30.3},
    {64, "Barton Miller",      5, 43.7},
    {67, "David Parter",       1, 99.9},
    {69, "Raghu Ramakrishnan", 9, 37.1},
    {71, "Guri Sohi",         10, 42.1},
    {73, "Prasoon Tiwari",     8, 39.2},
    {75, "Mary Vernon",        7, 43.1},
    {79, "David Wood",         3, 39.2},
    {84, "Mark Smucker",       9, 25.3},
    {87, "Martin Reames",     10, 24.1}
};



struct B {
   int  bid;
   char bname[12];
   char color[8];
}
Boats[] =
{
    {1, "Onion",      "white"},
    {2, "Buckey",     "red"  },
    {3, "Enterprise", "blue" },
    {4, "Voyager",    "green"},
    {5, "Wisconsin",  "red"  }
};


struct R {
   int   sid;
   int   bid;
   char  date[10];
}
Reserves[] =
{
    {10, 1, "05/10/95"},
    {21, 1, "05/11/95"},
    {10, 2, "05/11/95"},
    {31, 1, "05/12/95"},
    {10, 3, "05/13/95"},
    {69, 4, "05/12/95"},
    {69, 5, "05/14/95"},
    {21, 5, "05/16/95"},
    {57, 2, "05/10/95"},
    {35, 3, "05/15/95"}
};


Status s;



RelnScan::RelnScan(Reln reln, AttrType /*in*/[], int /*n_types*/)
{
   _reln = reln;
   index = 0;
   if (_reln == sailors)
   {
      count = 25;			// 25 sailors
      ConstructSailorsTuple(o_tuple);
   }
   else if (_reln == boats)
   {
      count = 5;
      ConstructBoatsTuple(o_tuple);
   }
   else
   {
      count = 10;
      ConstructReservesTuple(o_tuple);
   }

}


RelnScan::RelnScan(char * reln, AttrType /*in*/[], int /*n_types*/)
{
   std::cout << "WARNING: in file tests/source/joins.C\n";
   std::cout << "You are still using fake Scans!!!!\n";
   if (toupper(reln[0]) == 'S')      _reln = sailors;
   else if (toupper(reln[0]) == 'B') _reln = boats;
   else                              _reln = reserves;

   index = 0;
   if (_reln == sailors)
   {
      count = 25;			// 25 sailors
      ConstructSailorsTuple(o_tuple);
   }
   else if (_reln == boats)
   {
      count = 5;
      ConstructBoatsTuple(o_tuple);
   }
   else
   {
      count = 10;
      ConstructReservesTuple(o_tuple);
   }
}



RelnScan::~RelnScan()
{}


Status RelnScan::get_next(Tuple *&tuple)
{
   tuple = o_tuple;
   if (index == count) return DONE;
   switch (_reln)
   {
    case sailors:
      tuple->setFld(1, Sailors[index].sid);
      tuple->setFld(2, Sailors[index].sname);
      tuple->setFld(3, Sailors[index].rating);
      tuple->setFld(4, Sailors[index].age);
      break;

    case boats:
      tuple->setFld(1, Boats[index].bid);
      tuple->setFld(2, Boats[index].bname);
      tuple->setFld(3, Boats[index].color);
      break;

    case reserves:
      tuple->setFld(1, Reserves[index].sid);
      tuple->setFld(2, Reserves[index].bid);
      tuple->setFld(3, Reserves[index].date);
      break;
   }
   index++;
   return OK;
}



void RelnScan::ConstructSailorsTuple(Tuple *&t)
{
   AttrType types[] = {attrInteger, attrString, attrInteger, attrReal};
   short    sizes[] = {25};
   t = (Tuple *) new char [Tuple::max_size];
   t->setHdr(4, types, sizes);
}


void RelnScan::ConstructBoatsTuple(Tuple *&t)
{
   AttrType types[] = {attrInteger, attrString, attrString};
   short    sizes[] = {10, 5};
   t = (Tuple *) new char [Tuple::max_size];
   t->setHdr(3, types, sizes);
}


void RelnScan::ConstructReservesTuple(Tuple *&t)
{
   AttrType types[] = {attrInteger, attrInteger, attrString};
   short    sizes[] = {10};
   t = (Tuple *) new char [Tuple::max_size];
   t->setHdr(3, types, sizes);
}


JoinsTests::JoinsTests()
{}


void JoinsTests::Query1_CondExpr(CondExpr **&expr)
{
   expr = new CondExpr *[3];

   expr[0]        = new CondExpr;
   expr[0]->next  = NULL;
   expr[0]->op    = aopEQ;
   expr[0]->type1 = attrSymbol;
   expr[0]->operand1.symbol.relation = outer;
   expr[0]->operand1.symbol.offset   = 1;
   expr[0]->type2 = attrSymbol;
   expr[0]->operand2.symbol.relation = innerRel;
   expr[0]->operand2.symbol.offset   = 1;

   expr[1] = new CondExpr;
   expr[1]->op   = aopEQ;
   expr[1]->next = NULL;
   expr[1]->type1 = attrSymbol;
   expr[1]->operand1.symbol.relation = innerRel;
   expr[1]->operand1.symbol.offset   = 2;
   expr[1]->type2 = attrInteger;
   expr[1]->operand2.integer = 1;

   expr[2] = NULL;
}


void JoinsTests::Query2_CondExpr(CondExpr **&expr, CondExpr **&expr2)
{
   expr = new CondExpr * [2];

   expr[0]        = new CondExpr;
   expr[0]->next  = NULL;
   expr[0]->op    = aopEQ;
   expr[0]->type1 = attrSymbol;
   expr[0]->operand1.symbol.relation = outer;
   expr[0]->operand1.symbol.offset   = 1;
   expr[0]->type2 = attrSymbol;
   expr[0]->operand2.symbol.relation = innerRel;
   expr[0]->operand2.symbol.offset   = 1;

   expr[1] = NULL;


   expr2 = new CondExpr *[3];

   expr2[0]        = new CondExpr;
   expr2[0]->next  = NULL;
   expr2[0]->op    = aopEQ;
   expr2[0]->type1 = attrSymbol;
   expr2[0]->operand1.symbol.relation = outer;
   expr2[0]->operand1.symbol.offset   = 2;
   expr2[0]->type2 = attrSymbol;
   expr2[0]->operand2.symbol.relation = innerRel;
   expr2[0]->operand2.symbol.offset   = 1;

   expr2[1] = new CondExpr;
   expr2[1]->op   = aopEQ;
   expr2[1]->next = NULL;
   expr2[1]->type1 = attrSymbol;
   expr2[1]->operand1.symbol.relation = innerRel;
   expr2[1]->operand1.symbol.offset   = 3;
   expr2[1]->type2 = attrString;
   expr2[1]->operand2.string = "red";

   expr2[2] = NULL;
}

void JoinsTests::Disclaimer(void)
{
   std::cout << "\n\nAny resemblance of persons in this database to  people living or dead\n";
   std::cout << "is purely coincidental. The contents of this database do not  reflect\n";
   std::cout << "the views of the University, the Computer  Sciences Department or the\n";
   std::cout << "developers...\n\n";
}


void JoinsTests::Query3_CondExpr(CondExpr **&expr)
{
   expr = new CondExpr * [3];

   expr[0]        = new CondExpr;
   expr[0]->next  = NULL;
   expr[0]->op    = aopEQ;
   expr[0]->type1 = attrSymbol;
   expr[0]->operand1.symbol.relation = outer;
   expr[0]->operand1.symbol.offset   = 1;
   expr[0]->type2 = attrSymbol;
   expr[0]->operand2.symbol.relation = innerRel;
   expr[0]->operand2.symbol.offset   = 1;

   expr[1] = NULL;
   expr[2] = NULL;
}


void JoinsTests::Query4(void)
{
   // Sailors, Boats, Reserves Queries.

   std::cout << "Query: Find the names of sailors who have reserved a boat\n";
   std::cout << "       and print each name once\n\n";
   std::cout << "  SELECT DISTINCT S.sname\n";
   std::cout << "  FROM   Sailors S, Reserves R\n";
   std::cout << "  WHERE  S.sid = R.rid\n\n";

   CondExpr **outFilter;

   Query3_CondExpr(outFilter);

   Tuple *     t  = NULL;

   AttrType Stypes[] = {attrInteger, attrString, attrInteger, attrReal};
   short    Ssizes[] = {30};

   AttrType Rtypes[] = {attrInteger, attrInteger, attrString};
   short    Rsizes[] = {15};


   FldSpec  Sprojection[] = {{outer, 1}, {outer, 2}, {outer, 3}, {outer, 4}};
   
   CondExpr** selects = new CondExpr*[1];
   selects[0] = NULL;
   
   Iterator * am  = new FileScanIter("sailors", Stypes, Ssizes, 4, 4, 
				     Sprojection, selects, s);
   

   
   

  FldSpec  Rprojection[] = {{outer, 1}, {outer, 2}, {outer, 3}};


  Iterator * am2 = new FileScanIter("reserves", Rtypes, Rsizes, 3, 3,
				    Rprojection, selects, s);


   FldSpec  proj_list[] = {{outer, 2}};
   AttrType jtype[]     = { attrString };
   short    jsizes[]    = {30};

   sort_merge sm(Stypes, 4, Ssizes,
		 Rtypes, 3, Rsizes,
		 1, sizeof(int),
		 1, sizeof(int),
		 10,
		 am, am2,
		 FALSE, FALSE, Ascending,
		 outFilter, proj_list, 1, s);

   DuplElim ed(jtype, 1, jsizes, (Iterator *) &sm, 10, FALSE, s);
   Iterator *result = &ed;

   while (result->get_next(t) != DONE)
      t->print(jtype);

   std::cout << std::endl << std::endl;
   delete am;
   return;
}



void JoinsTests::Query5_CondExpr(CondExpr **&expr2)
{
   expr2 = new CondExpr * [3];

   expr2[0]        = new CondExpr;
   expr2[0]->next  = NULL;
   expr2[0]->op    = aopEQ;
   expr2[0]->type1 = attrSymbol;
   expr2[0]->operand1.symbol.relation = outer;
   expr2[0]->operand1.symbol.offset   = 1;
   expr2[0]->type2 = attrSymbol;
   expr2[0]->operand2.symbol.relation = innerRel;
   expr2[0]->operand2.symbol.offset   = 1;

   expr2[1] = new CondExpr;
   expr2[1]->op   = aopGT;
   expr2[1]->next = NULL;
   expr2[1]->type1 = attrSymbol;
   expr2[1]->operand1.symbol.relation = outer;
   expr2[1]->operand1.symbol.offset   = 4;		// age
   expr2[1]->type2 = attrReal;
   expr2[1]->operand2.real = 40.0;

   expr2[1]->next = new CondExpr;
   expr2[1]->next->op   = aopLT;
   expr2[1]->next->next = NULL;
   expr2[1]->next->type1 = attrSymbol;
   expr2[1]->next->operand1.symbol.relation = outer;
   expr2[1]->next->operand1.symbol.offset   = 3;	// rating
   expr2[1]->next->type2 = attrInteger;
   expr2[1]->next->operand2.integer = 7;

   expr2[2] = NULL;
}


void JoinsTests::Query5(void)
{
   // Sailors, Boats, Reserves Queries.

   std::cout << "Query: Find the names of old sailors or sailors with a rating less\n";
   std::cout << "       than 7, who have reserved a boat, (perhaps to increase the\n";
   std::cout << "       amount they have to pay to make a reservation\n\n";
   std::cout << "  SELECT S.sname, S.rating, S.age\n";
   std::cout << "  FROM   Sailors S, Reserves R\n";
   std::cout << "  WHERE  S.sid = R.rid and (S.age > 40 || S.rating < 7)\n\n";

   CondExpr **outFilter;

   Query5_CondExpr(outFilter);

   Tuple *     t  = NULL;

   AttrType Stypes[] = {attrInteger, attrString, attrInteger, attrReal};
   short    Ssizes[] = {30};

   AttrType Rtypes[] = {attrInteger, attrInteger, attrString};
   short    Rsizes[] = {15};




   FldSpec  Sprojection[] = {{outer, 1}, {outer, 2}, {outer, 3}, {outer, 4}};

   CondExpr** selects = new CondExpr*[1];
   selects[0] = NULL;

   Iterator * am  = new FileScanIter("sailors", Stypes, Ssizes, 4, 4, 
				     Sprojection, selects, s);


   FldSpec  Rprojection[] = {{outer, 1}, {outer, 2}, {outer, 3}};
   
   
   Iterator * am2 = new FileScanIter("reserves", Rtypes, Rsizes, 3, 3,
				     Rprojection, selects, s);
   


   FldSpec  proj_list[] = {{outer, 2}, {outer, 3}, {outer, 4}};
   AttrType jtype[]     = { attrString, attrInteger, attrReal };

   sort_merge sm(Stypes, 4, Ssizes,
		 Rtypes, 3, Rsizes,
		 1, sizeof(int),
		 1, sizeof(int),
		 10,
		 am, am2,
		 FALSE, FALSE, Ascending,
		 outFilter, proj_list, 3, s);

   Iterator *result = &sm;

   while (result->get_next(t) != DONE)
      t->print(jtype);

   std::cout << std::endl << std::endl;
   delete am;
   return;
}

void JoinsTests::Query1(void)
{
  Status s;
   // Sailors, Boats, Reserves Queries.

   std::cout << "Query: Find the names of sailors who have reserved boat number 1\n\n";
   std::cout << "  SELECT S.sname\n";
   std::cout << "  FROM   Sailors S, Reserves R\n";
   std::cout << "  WHERE  S.sid = R.sid AND R.bid = 1\n\n";

   CondExpr **outFilter;

   Query1_CondExpr(outFilter);

   Tuple *     t  = NULL;


   AttrType Stypes[] = {attrInteger, attrString, attrInteger, attrReal};
   short    Ssizes[] = {30};
   FldSpec  Sprojection[] = {{outer, 1}, {outer, 2}, {outer, 3}, {outer, 4}};

   CondExpr** selects = new CondExpr*[1];
   selects[0] = NULL;

   Iterator * am  = new FileScanIter("sailors", Stypes, Ssizes, 4, 4, 
				     Sprojection, selects, s);

  AttrType Rtypes[] = {attrInteger, attrInteger, attrString};
  short    Rsizes[] = {15};
  FldSpec  Rprojection[] = {{outer, 1}, {outer, 2}, {outer, 3}};


  Iterator * am2 = new FileScanIter("reserves", Rtypes, Rsizes, 3, 3,
				    Rprojection, selects, s);

   FldSpec  proj_list[] = {{outer, 2}};
   AttrType jtype[]     = { attrString };

   sort_merge sm(Stypes, 4, Ssizes,
		 Rtypes, 3, Rsizes,
		 1, sizeof(int),
		 1, sizeof(int),
		 10,
		 am, am2,
		 FALSE, FALSE, Ascending,
		 outFilter, proj_list, 1, s);

   Iterator *result = &sm;

   while (result->get_next(t) != DONE)
      t->print(jtype);


   std::cout << std::endl << std::endl;
   delete am;
   return;

}





void JoinsTests::Query2(void)
{
   // Sailors, Boats, Reserves Queries.
   std::cout << "Query: Find the names of sailors who have reserved a red boat\n";
   std::cout << "       and return then in alphabetical order.\n\n";
   std::cout << "  SELECT   S.sname\n";
   std::cout << "  FROM     Sailors S, Boats B, Reserves R\n";
   std::cout << "  WHERE    S.sid = R.rid AND R.bid = B.bid AND B.color = 'red'\n";
   std::cout << "  ORDER BY S.sname\n\n";

   std::cout << "Plan used:\n";
   std::cout << " Sort (Pi(sname) (B  |><|  Pi(sname, bid) (S  |><|  R)))\n\n";

   CondExpr **outFilter, **outFilter2;
   Query2_CondExpr(outFilter, outFilter2);
   Tuple *     t  = NULL;

   AttrType Stypes[] = {attrInteger, attrString, attrInteger, attrReal};
   short    Ssizes[] = {30};
   AttrType Rtypes[] = {attrInteger, attrInteger, attrString};
   short    Rsizes[] = {15};
   AttrType Btypes[] = {attrInteger, attrString, attrString};
   short    Bsizes[] = {30, 20};
   AttrType Jtypes[] = {attrString, attrInteger};
   short    Jsizes[] = {30};
   AttrType JJtype[] = {attrString};
   short    JJsize[] = {30};

   FldSpec  proj1[]  = {{outer, 2}, {innerRel, 2}};	// S.sname, R.bid
   FldSpec  proj2[]  = {{outer, 1}};


   FldSpec  Sprojection[] = {{outer, 1}, {outer, 2}, {outer, 3}, {outer, 4}};

   CondExpr** selects = new CondExpr*[1];
   selects[0] = NULL;

   Iterator * am = new FileScanIter("sailors", Stypes, Ssizes, 4, 4, 
				     Sprojection, selects, s);

   nested_loops_join* nlj = new nested_loops_join(Stypes, 4, Ssizes,
						  Rtypes, 3, Rsizes,
						  10,
						  am, "reserves",
						  outFilter, NULL, proj1, 2, s);

   nested_loops_join* nlj2 = new nested_loops_join(Jtypes, 2, Jsizes,
						   Btypes, 3, Bsizes,
						   10,
						   (Iterator *)nlj, "boats",
						   outFilter2, NULL, proj2, 1, s);
   Sort sort_names(JJtype, 1, JJsize,
		   (Iterator *) nlj2, 1, Ascending, JJsize[0], 10, s);
   
   //  Iterator *result = &sort_names;
   Iterator * result = nlj2;
   
   while (result->get_next(t) != DONE)
     t->print(JJtype);

   std::cout << std::endl << std::endl;
   delete nlj2; // this recursively deletes nlj and am : BUG FIX Ranjani R..
   return;
 }





void JoinsTests::Query3(void)
{
   // Sailors, Boats, Reserves Queries.

   std::cout << "Query: Find the names of sailors who have reserved a boat\n\n";
   std::cout << "  SELECT S.sname\n";
   std::cout << "  FROM   Sailors S, Reserves R\n";
   std::cout << "  WHERE  S.sid = R.rid\n\n";

   CondExpr **outFilter;

   Query3_CondExpr(outFilter);

   Tuple *     t  = NULL;

   AttrType Stypes[] = {attrInteger, attrString, attrInteger, attrReal};
   short    Ssizes[] = {30};

   AttrType Rtypes[] = {attrInteger, attrInteger, attrString};
   short    Rsizes[] = {15};

   FldSpec  Sprojection[] = {{outer, 1}, {outer, 2}, {outer, 3}, {outer, 4}};

   CondExpr** selects = new CondExpr*[1];
   selects[0] = NULL;

   Iterator * am  = new FileScanIter("sailors", Stypes, Ssizes, 4, 4, 
				     Sprojection, selects, s);


   FldSpec  Rprojection[] = {{outer, 1}, {outer, 2}, {outer, 3}};
   
   
   Iterator * am2 = new FileScanIter("reserves", Rtypes, Rsizes, 3, 3,
				     Rprojection, selects, s);
   

   FldSpec  proj_list[] = {{outer, 2}};
   AttrType jtype[]     = { attrString };

   sort_merge sm(Stypes, 4, Ssizes,
		 Rtypes, 3, Rsizes,
		 1, sizeof(int),
		 1, sizeof(int),
		 10,
		 am, am2,
		 FALSE, FALSE, Ascending,
		 outFilter, proj_list, 1, s);

   Iterator *result = &sm;

   while (result->get_next(t) != DONE)
      t->print(jtype);

   std::cout << std::endl << std::endl;
   delete am;
   return;
}

