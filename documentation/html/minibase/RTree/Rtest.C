// Rtest.C
//
// Armand Zakarian
// May 1995
//
// Test driver for R-tree code
// 
//
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <ostream>
#include <assert.h>
#include <math.h>

#include "buf.h"
#include "db.h"
#include "new_error.h"
#define	MAX_COMMAND_SIZE 100

class trans_lock_info;
class RecInfo;
class ShMemMgr;
class Xaction_manager;

DB* 	db;
char Dbname[MAX_COMMAND_SIZE];
ShMemMgr* 	SM_Mgr;
SpaceMgr*  space_mgr;
Xaction_manager* XMgr;
BufMgr* 	bm;

global_error minirel_error;
int 	Xid;
lsn_t	invalid_lsn;

#include "Rtree.h"


#define NUMTESTS 	6


void test1(char *name, int i)
{
  std::cout << "***** Inserting tuples...\n" ;
  Status s ;
  RtreeFile R(s, name) ;
  assert(s==OK) ;

  // int i=0 ;
  RID rid ;
  float box[2][2] ;
  while(cin>>box[0][0]>>box[0][1]>>box[1][0]>>box[1][1]) {
    rid.pageNo = i/100 ;
    rid.slotNo = i%100 ;
    if(R.insert(box, rid)!=OK) {
      minirel_error.show_errors() ;
      return ;
    }
    std::cout<<"("<<rid.pageNo<<", "<<rid.slotNo<<")---> "<<
      box[0][0]<<" "<<box[0][1]<<" "<<box[1][0]<<" "<<box[1][1]<<'\n' ;
    ++i ;
  }
}
    
void test2(char *name)
{
  std::cout << "***** Dumping contents of index...\n" ;
  Status s ;
  RtreeFile R(s, name) ;
  assert(s==OK) ;

  int i=0 ;
  RID rid ;
  IndexFileScan *Scan = R.new_scan(0) ;
  while((s=Scan->get_next(rid))==OK) {
    std::cout<<"("<<rid.pageNo<<", "<<rid.slotNo<<")---> "<<'\n' ;
  }
  // assert(s==DONE) ;
  if(s!=DONE) minirel_error.show_errors() ;
  delete Scan ;
}

void test3(char *name, float box[2][2])
{
  std::cout << "***** Scanning for a box...\n" ;
  Status s ;
  RtreeFile R(s, name) ;
  assert(s==OK) ;

  int i=0 ;
  RID rid ;
  IndexFileScan *Scan = R.new_scan(box) ;
  while((s=Scan->get_next(rid))==OK) {
    std::cout<<"("<<rid.pageNo<<", "<<rid.slotNo<<")---> "<<'\n' ;
  }
  // assert(s==DONE) ;
  if(s!=DONE) minirel_error.show_errors() ;
  delete Scan ;
}

void test4(char *name, float box[2][2])
{
  std::cout << "***** Deleting tuples touching a box...\n" ;
  Status s ;
  RtreeFile R(s, name) ;
  assert(s==OK) ;

  int i=0 ;
  RID rid ;
  IndexFileScan *Scan = R.new_scan(box) ;
  while((s=Scan->get_next(rid))==OK) {
    // std::cout<<"("<<rid.pageNo<<", "<<rid.slotNo<<")---> "<<'\n' ;
    if(Scan->delete_current()!=OK) {
      minirel_error.show_errors() ;
      return ;
    }
  }
  // assert(s==DONE) ;
  delete Scan ;
}

void test5(char *name)
{
  std::cout << "***** Printing root page...\n" ;
  Status s ;
  RtreeFile R(s, name) ;
  assert(s==OK) ;
  R.print_root() ;
}

void test6(char *name)
{
  std::cout << "***** Destroying index...\n" ;
  Status s ;
  RtreeFile R(s, name) ;
  assert(s==OK) ;
  if(R.destroy()!=OK) minirel_error.show_errors() ;
}


void usage() 
{
  std::cout<<"usage: Rtest <test_no> <index_name> [<box>] [<base_rid>]\n"
      <<"           Test 1:  read boxes from stdin and insert in index\n"
      <<"           Test 2:  dump contents of index to stdout\n"
      <<"           Test 3:  scan index for entries intersecting the given\
 <box>\n"
      <<"           Test 4:  scan and delete entries intersecting the given\
 <box>\n"
      <<"           Test 5:  print root page\n"
      <<"           Test 6:  destroy index\n"
      <<"           Test 10+i:  Reinitialize DB, then perform Test i\n" ;

}




main(int argc, char *argv[]) 
{

  strcpy(Dbname, "Rtest.db") ;

  if(argc==1) {
    usage() ;
    
  } else {


    Status s ;
    int test = atoi(argv[1]) ;
    int newdb = test/10 ;
    test = test%10 ;

    SM_Mgr = new ShMemMgr(1000);
    if(newdb) db=new DB(Dbname, 1500, s); 
    else db=new DB(Dbname, s) ;
    assert(s==OK) ;

    if(test<1 || test>NUMTESTS) {
      usage() ;
    } else {

      if(test==1) {
        test1(argv[2], atoi(argv[3])) ;
      }
      if(test==2) test2(argv[2]) ;
      if(test==3) {
        float box[2][2] ;
        box[0][0] = atof(argv[3]) ;
        box[0][1] = atof(argv[4]) ;
        box[1][0] = atof(argv[5]) ;
        box[1][1] = atof(argv[6]) ;
        test3(argv[2], box) ;
      }
      if(test==4) {
        float box[2][2] ;
        box[0][0] = atof(argv[3]) ;
        box[0][1] = atof(argv[4]) ;
        box[1][0] = atof(argv[5]) ;
        box[1][1] = atof(argv[6]) ;
        test4(argv[2], box) ;
      }
      if(test==5) test5(argv[2]) ;
      if(test==6) test6(argv[2]) ;
    }
    delete SM_Mgr;
    delete db;
  }
}

