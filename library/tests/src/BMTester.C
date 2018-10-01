#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <ostream>
#include <assert.h>
#include <unistd.h>

#include "buf.h"
#include "db.h"
#include <pwd.h>


#include "BMTester.h"

//extern "C" int getpid();
//extern "C" int unlink( const char* );


BMTester::BMTester() : TestDriver( "buftest" )
{}


BMTester::~BMTester()
{}


int BMTester::test1()
{
    std::cout << "\n  Test 1 does a simple test of normal buffer manager operations:\n";

      // We choose this number to ensure that at least one page will have to be
      // written during this test.
    unsigned numPages = MINIBASE_BM->getNumUnpinnedBuffers() + 1;
    Page* pg;
    PageId pid, firstPid, lastPid;

    std::cout << "  - Allocate a bunch of new pages\n";
    Status status = MINIBASE_BM->newPage( firstPid, pg, numPages );
    if ( status != OK )
      {
        std::cerr << "*** Could not allocate " << numPages << " new pages in the database.\n";
//        MINIBASE_XACTMGR->abort();
        return FALSE;
      }

      // Unpin that first page... to simplify our loop.
    status = MINIBASE_BM->unpinPage(firstPid);
    if ( status != OK )
        std::cerr << "*** Could not unpin the first new page.\n";


    std::cout << "  - Write something on each one\n";
    for ( pid=firstPid, lastPid=pid+numPages;
          status == OK && pid < lastPid; ++pid )
      {
        status = MINIBASE_BM->pinPage( pid, pg, /*emptyPage:*/ TRUE );
        if ( status != OK )
            std::cerr << "*** Could not pin new page " << pid << std::endl;

        if ( status == OK )
          {
              // Copy the page number + 99999 onto each page.  It seems
              // unlikely that this bit pattern would show up there by
              // coincidence.
            int data = pid + 99999;
            memcpy( (void*)pg, &data, sizeof data );

            status = MINIBASE_BM->unpinPage( pid, /*dirty:*/ TRUE );
            if ( status != OK )
                std::cerr << "*** Could not unpin dirty page " << pid << std::endl;
          }
      }


    if ( status == OK )
        std::cout << "  - Read that something back from each one\n"
             << "      (because we're buffering, this is where most of the writes happen)\n";
    for ( pid=firstPid; status == OK && pid < lastPid; ++pid )
      {
        status = MINIBASE_BM->pinPage( pid, pg, /*emptyPage:*/ FALSE );
        if ( status != OK )
            std::cerr << "*** Could not pin page " << pid << std::endl;

        if ( status == OK )
          {
            int data;
            memcpy( &data, (void*)pg, sizeof data );

            if ( data != pid + 99999 )
              {
                status = FAIL;
                std::cerr << "*** Read wrong data back from page " << pid << std::endl;
              }

            status = MINIBASE_BM->unpinPage( pid, /*dirty:*/ FALSE );
            if ( status != OK )
                std::cerr << "*** Could not unpin page " << pid << std::endl;
          }
      }


    if ( status == OK ) std::cout << "  - Free the pages again\n";
    for ( pid=firstPid; pid < lastPid; ++pid )
      {
        Status st2 = MINIBASE_BM->freePage( pid );
        if ( status == OK && st2 != OK )
          {
            status = st2;
            std::cerr << "*** Error freeing page " << pid << std::endl;
          }
      }


    if ( status == OK )
        std::cout << "  Test 1 completed successfully.\n";

    return status == OK;
}



int BMTester::test2()
{
    std::cout << "\n  Test 2 exercises some illegal buffer manager operations:\n";

    minibase_errors.clear_errors();

      // We choose this number to ensure that pinning this number of buffers
      // should fail.
    unsigned numPages = MINIBASE_BM->getNumUnpinnedBuffers() + 1;
    Page* pg;
    PageId pid, firstPid, lastPid;


    std::cout << "  - Try to pin more pages than there are frames\n";
    Status status = MINIBASE_BM->newPage( firstPid, pg, numPages );
    if ( status != OK )
      {
        std::cerr << "*** Could not allocate " << numPages << " new pages in the database.\n";
//        MINIBASE_XACTMGR->abort();
        return FALSE;
      }

      // First pin enough pages that there is no more room.
    for ( pid=firstPid+1, lastPid=firstPid+numPages-1;
          status == OK && pid < lastPid; ++pid )
      {
        status = MINIBASE_BM->pinPage( pid, pg, /*emptyPage:*/ TRUE );
        if ( status != OK )
            std::cerr << "*** Could not pin new page " << pid << std::endl;
      }

      // Make sure the buffer manager thinks there's no more room.
    if ( status == OK  &&  MINIBASE_BM->getNumUnpinnedBuffers() != 0 )
      {
        status = FAIL;
        std::cerr << "*** The buffer manager thinks it has "
             << MINIBASE_BM->getNumUnpinnedBuffers() << " available frames,\n"
             << "    but it should have none.\n";
      }

      // Now pin that last page, and make sure it fails.
    if ( status == OK )
      {
        status = MINIBASE_BM->pinPage( lastPid, pg, /*emptyPage:*/ TRUE );
        testFailure( status, BUFMGR, "Pinning too many pages" );
      }




    if ( status == OK )
      {
        status = MINIBASE_BM->pinPage( firstPid, pg );
        if ( status != OK )
            std::cerr << "*** Could not acquire a second pin on a page\n";
      }
    if ( status == OK )
      {
        std::cout << "  - Try to free a doubly-pinned page\n";
        status = MINIBASE_BM->freePage( firstPid );
        testFailure( status, BUFMGR, "Freeing a pinned page" );
        MINIBASE_BM->unpinPage( firstPid );
      }




    if ( status == OK )
      {
        std::cout << "  - Try to unpin an unpinned page\n";
        status = MINIBASE_BM->unpinPage( lastPid );
        testFailure( status, BUFMGR, "Unpinning an unpinned page" );
      }



    for ( pid=firstPid; pid <= lastPid; ++pid )
      {
        Status st2 = MINIBASE_BM->freePage( pid );
        if ( status == OK && st2 != OK )
          {
            status = st2;
            std::cerr << "*** Error freeing page " << pid << std::endl;
          }
      }



    if ( status == OK )
        std::cout << "  Test 2 completed successfully.\n";

    return status == OK;
}


int BMTester::test3()
{
    std::cout << "\n  Test 3 exercises some of the internals of the buffer manager\n";

    unsigned index, numPages = NUMBUF + 10;
    Page* pg;
    PageId pid, pids[numPages];
    Status status = OK;

    std::cout << "  - Allocate and dirty some new pages, one at a time, and leave some pinned\n";
    for ( index=0; status == OK && index < numPages; ++index )
      {
        status = MINIBASE_BM->newPage( pid, pg );
        if ( status == OK )
            pids[index] = pid;
        else
            std::cerr << "*** Could not allocate new page number " << index+1 << std::endl;

        if ( status == OK )
          {
              // Copy the page number + 99999 onto each page.  It seems
              // unlikely that this bit pattern would show up there by
              // coincidence.
            int data = pid + 99999;
            memcpy( (void*)pg, &data, sizeof data );


              // Leave the page pinned if it equals 12 mod 20.  This is a
              // random number based loosely on a bug report.
            if ( pid % 20 != 12 )
                status = MINIBASE_BM->unpinPage( pid, /*dirty:*/ true );
            if ( status != OK )
                std::cerr << "*** Could not unpin dirty page " << pid << std::endl;
          }
      }


    if ( status == OK )
        std::cout << "  - Read the pages\n";
    for ( index=0; status == OK && index < numPages; ++index )
      {
        pid = pids[index];
        status = MINIBASE_BM->pinPage( pid, pg );
        if ( status != OK )
            std::cerr << "*** Could not pin page " << pid << std::endl;

        if ( status == OK )
          {
            int data;
            memcpy( &data, (void*)pg, sizeof data );

            if ( data != pid + 99999 )
              {
                status = FAIL;
                std::cerr << "*** Read wrong data back from page " << pid << std::endl;
              }
          }
        if ( status == OK )
          {
            status = MINIBASE_BM->unpinPage( pid );
            if ( status != OK )
                std::cerr << "*** Could not unpin page " << pid << std::endl;
          }
        if ( status == OK && (pid % 20 == 12) )
          {
            status = MINIBASE_BM->unpinPage( pid, /*dirty:*/ true );
            if ( status != OK )
                std::cerr << "*** Could not unpin page " << pid << std::endl;
          }
      }


    if ( status == OK )
        std::cout << "  Test 3 completed successfully.\n";

    return status == OK;
}

int BMTester::test4()
{
    return true;
}

int BMTester::test5()
{
    return true;
}

int BMTester::test6()
{
    return true;
}



const char* BMTester::testName()
{
    return "Buffer Management";
}


void BMTester::runTest( Status& status, TestDriver::testFunction test )
{
    minibase_globals = new SystemDefs( status, dbpath, logpath, 
				  NUMBUF+20, 500, NUMBUF, "Clock" );
    if ( status == OK )
      {
        TestDriver::runTest(status,test);

#ifdef    MULTIUSER
        status = MINIBASE_XACTMGR->commit();
        if( status != OK) {
            std::cerr << "Failure to commit the transaction\n" << std::endl;
        }
#endif

        delete minibase_globals, minibase_globals = 0;
      }

    char* newdbpath;
    char* newlogpath;
    char remove_logcmd[50];
    char remove_dbcmd[50];

    newdbpath = new char[ strlen(dbpath) + 20];
    newlogpath = new char[ strlen(logpath) + 20];
    strcpy(newdbpath,dbpath);
	strcpy(newlogpath, logpath);

#ifdef MULTIUSER
	pwd = getpwuid(getuid());
	sprintf(remove_dbcmd, "/bin/rm -rf %s-%s", dbpath,pwd->pw_name);
	sprintf(remove_logcmd, "/bin/rm -rf %s-%s",logpath, pwd->pw_name);
#else
    sprintf(remove_logcmd, "/bin/rm -rf %s", logpath);
	sprintf(remove_dbcmd, "/bin/rm -rf %s", dbpath);
#endif
   system(remove_logcmd);
   system(remove_dbcmd);
#ifdef MULTIUSER
	if ( (pwd = getpwuid(getuid())) != NULL) {
      sprintf( newdbpath, "%s-%s", dbpath, pwd->pw_name );
	  sprintf( newlogpath, "%s-%s", logpath, pwd->pw_name );
  	}
#else
	sprintf(newdbpath, "%s", dbpath);
	sprintf(newlogpath, "%s", logpath);
#endif


    unlink( newdbpath );
    unlink( newlogpath );

    delete newdbpath; delete newlogpath;

}


Status BMTester::runTests()
{
    return TestDriver::runTests();
}


Status BMTester::runAllTests()
{
#ifdef MULTIUSER
    std::cout << " *** These tests do NOT tackle concurrency or recovery ***\n";
#endif
    return TestDriver::runAllTests();
}
