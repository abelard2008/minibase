// test program for linearhashing in MINIREL
// Weiqing Huang, whuang@cs.wisc.edu, May 1995

// $Id: linearHash.C,v 1.5 1996/06/02 03:07:13 luke Exp $

#include <strstream>
#include <ostream>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <strings.h>

#include "buf.h"
#include "db.h"
/*  #include "linearhashing.h"  */
#include "lHtests.h"



extern SystemDefs* minibase_globals;

// #define MAX_COMMAND_SIZE 100

LinearHashTests::LinearHashTests()
{}


Status LinearHashTests::runTests()
{
  test1();
  std::cout<<"Hit a return to continue\n"<< std::endl; fflush(stdout);
  getchar();
  return OK;
}

/*
void LinearHashTests::gen_rec(void * rec, int key, int len)
{
  keysamples.getline(rec, len); 
  rid.pageNo = (int)(key[0]+key[1]+key[2]);
  rid.slotNo = rid.pageNo;
}
*/


void LinearHashTests::gen_rec(char * rec, int key, int len)
{

bzero(rec, len);
rec[0] = ' ';
rec[1] = 'r';
rec[2] = 'e';
rec[3] = 'c';
rec[4] = 'o';
rec[5] = 'r';
rec[6] = 'd';
rec[7] = ' ';
rec[8] = '0';
rec[9] = '0';
rec[10] = '0';
rec[11] = '0';
rec[12] = '\0';

if (key > 999)
{
 rec[8] = (key / 1000) + 48;
 key -= (key / 1000) * 1000;
}
if (key > 99)
{
 rec[9] = (key / 100) + 48;
 key -= (key / 100) * 100;
}
if (key > 9)
{
 rec[10] = (key / 10) + 48;
 key -= (key / 10) * 10;
}
rec[11] = key + 48;
}

void LinearHashTests::test1(void)
{

    std::cout << "\nBeginning of test 1:\n";
    std::cout << "Create database named LHDRIVER and insert 1000 key - rid pairs into it.\n";

    Status status;
/*
    LinearHashingFile *lhf;
    int i, key;
    RID rid;
    char * rec = new char[100];
    
    system("/bin/rm -rf /tmp/lhlog /tmp/LHDRIVER");
    minibase_globals = new SystemDefs(status,"/tmp/LHDRIVER", "/tmp/lhlog",
				      1000,500,200,"Clock");

    // create new index
    lhf = new LinearHashingFile(status, "LinearHashing", 100,attrString);
    if (status != OK) {
        minibase_errors.show_errors();
        exit(1);
    }

    // insert records
    for (i = 0; i < 100; i++) {
        rid.pageNo = i; rid.slotNo = i+1;
        key = i % 300;
        gen_rec(rec, key, 100);
        std::cout << "inserting entry: " << rec << std::endl;
        if ((status = lhf->insert(rec, rid)) != OK) {
            minibase_errors.show_errors();
        }
    }

    std::cout << "all records have been inserted successfully.\n";
    std::cout << "End of test 1\n\n";

    MINIBASE_BM->printSelf();

    delete rec;    
    delete lhf;

    delete minibase_globals;

    system("/bin/rm -rf /tmp/lhlog /tmp/LHDRIVER");
*/
    std::cout << "Done with tests...\n";
}

