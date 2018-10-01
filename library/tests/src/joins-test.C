
#include <stdio.h>

#include "test.h"
#include "minirel.h"
#include "tuple.h"
#include "iterator.h"
#include "joins.h"
#include "sort.h"
#include "projection.h"
#include "joins_tests.h"

#include "buf.h"
#include "db.h"



void error(char * s)
{
   fprintf(stderr, "%s", s);
   exit(1);
}

// ========================================================================
//
// Here is the implementation for the tests. There are N tests performed.
// We start off by showing that each operator works on its own.
// Then more complicated trees are constructed.
// As a nice feature, we allow the user to specify a selection condition.
// We also allow the user to hardwire trees together.
//


// int raghu_main();

Status JoinsTests::runTests()	// probably want to give the test numbers here.
{
    Status status;
   minibase_globals = new SystemDefs( status, "/tmp/minibase.jointestdb", 100 );

   Disclaimer();

   Query1();
   Query2();
   Query3();
   Query4();
   Query5();

   delete minibase_globals;

   std::cout << "Finished joins testing\n" << std::endl;
   fflush(stdout);

   return OK;
}


