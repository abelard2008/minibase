#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <ostream>
#include <assert.h>
#include <unistd.h>
#include "db.h"
#include "buf.h"
#include "db_driver.h"


DBDriver::DBDriver() : TestDriver( "dbtest" )
{}

DBDriver::~DBDriver()
{}


Status DBDriver::runTests()
{
    return TestDriver::runTests();
}


int DBDriver::test1()
{
    std::cout << "\n  Test 1 creates a new database and does some tests of "
        "normal operations:\n";
 
    Status status;
    minibase_globals = new SystemDefs( status, dbpath, logpath, 100,
                                       600, 100, "Clock" );
    PageId pgid = 0;


    std::cout << "  - Add some file entries\n";

    for ( int i=0; i < 6 && status == OK; ++i )
      {
        char name[15];
        sprintf(name, "file %d",i);
        status = MINIBASE_DB->allocate_page(pgid, 1);
        if ( status == OK )
            status = MINIBASE_DB->add_file_entry(name,pgid);
        if ( status != OK )
            std::cerr << "*** Could not add file entry " << name << std::endl;
      }

    
    if ( status == OK )
      {
        std::cout << "  - Allocate a run of pages\n";
        status = MINIBASE_DB->allocate_page( runStart, 30 );
        if ( status != OK )
            std::cerr << "*** Could not allocate a run of pages\n";
      }


    if ( status == OK )
      {
        std::cout << "  - Write something on some of them\n";
        for ( int i=0; i<20 && status == OK; ++i )
          {
            Page pg;
            ((char*)&pg)[0] = 'A'+i;
            status = MINIBASE_DB->write_page(runStart+i,&pg);
            if ( status != OK )
                std::cerr << "*** Error writing to page " << runStart + i << std::endl;
          }
      }


    if ( status == OK )
      {
        std::cout << "  - Deallocate the rest of them\n";
        status = MINIBASE_DB->deallocate_page( runStart+20, 10 );
        if ( status != OK )
            std::cerr << "*** Error deallocating pages\n";
      }

//<<<<<<< db_driver.C
#ifdef MULTIUSER
//=======

#ifdef    MULTIUSER
//>>>>>>> 1.10
    status = MINIBASE_XACTMGR->commit();
    if( status != OK) {
	std::cerr << "Failure to commit the transaction\n" << std::endl;
    }
//<<<<<<< db_driver.C
#endif
//=======
#endif 
//>>>>>>> 1.11

    delete minibase_globals;

    if ( status == OK )
        std::cout << "  Test 1 completed successfully.\n";

    return (status == OK);
}



int DBDriver::test2()
{
    std::cout << "\n  Test 2 opens the database created in test 1 and does some further tests:\n";

    Status status;
    minibase_globals = new SystemDefs( status, dbpath, logpath, 0, 500, 100, "Clock" );

    PageId pgid = 0;

    
    std::cout << "  - Delete some of the file entries\n";
    for ( int i=0; i<3 && status == OK; ++i )
      {
        char name[15];
        sprintf(name, "file %d",i);
        status = MINIBASE_DB->delete_file_entry(name);
        if ( status != OK )
            std::cerr << "*** Could not delete file entry " << name << std::endl;
      }


    if ( status == OK )
      {
        std::cout << "  - Look up file entries that should still be there\n";
        for ( int i=3; i<6 && status == OK; ++i )
          {
            char name[15];
            sprintf(name, "file %d",i);
            status = MINIBASE_DB->get_file_entry(name, pgid);
            if ( status != OK )
                std::cerr << "*** Could not find file entry " << name << std::endl;
          }
      }


    if ( status == OK )
      {
        std::cout << "  - Read stuff back from pages we wrote in test 1\n";

        for ( int i=0; i<20 && status == OK; ++i )
          {
            Page pg;
            status = MINIBASE_DB->read_page(runStart+i,&pg);
            if ( status != OK )
                std::cerr << "*** Error reading from page " << runStart+i << std::endl;
            else if ( ((char*)&pg)[0] != 'A' + i )
              {
                status = FAIL;
                std::cerr << "*** Data read does not match what was written on page "
                     << runStart+i << std::endl;
              }
          }
      }

//<<<<<<< db_driver.C
#ifdef MULTIUSER
//=======

#ifdef    MULTIUSER
//>>>>>>> 1.10
    status = MINIBASE_XACTMGR->commit();
    if( status != OK) {
	std::cerr << "Failure to commit the transaction\n" << std::endl;
    }
//<<<<<<< db_driver.C
#endif
//=======
#endif

//>>>>>>> 1.11
    delete minibase_globals;

    if ( status == OK )
        std::cout << "  Test 2 completed successfully.\n";

    return (status == OK);
}



int DBDriver::test3()
{
    std::cout << "\n  Test 3 tests for some error conditions:\n";

    Status status;
    minibase_globals = new SystemDefs( status, dbpath, logpath, 0, 500, 100, "Clock" );

    PageId pgid = 0;

    
    if ( status == OK )
      {
        std::cout << "  - Look up a deleted file entry\n";
        status = MINIBASE_DB->get_file_entry("file 1", pgid);
        testFailure( status, FAIL, "Looking up a deleted file entry", FALSE );
      }


    if ( status == OK )
      {
        std::cout << "  - Try to delete a nonexistent file entry\n";
        status = MINIBASE_DB->delete_file_entry("blargle");
        testFailure( status, DBMGR, "Deleting a nonexistent file entry" );
      }


    if ( status == OK )
      {
        std::cout << "  - Look up a nonexistent file entry\n";
        status = MINIBASE_DB->get_file_entry("blargle", pgid);
        testFailure( status, FAIL, "Looking up a nonexistent file entry", FALSE );
      }


    if ( status == OK )
      {
        std::cout << "  - Try to add a file entry that's already there\n";
        status = MINIBASE_DB->add_file_entry("file 3",runStart);
        testFailure( status, DBMGR, "Adding a duplicate file entry" );
      }

    
    if ( status == OK )
      {
        std::cout << "  - Try to add a file entry whose name is too long\n";
        char name[ MAX_NAME + 2 ];
        memset( name, 'x', MAX_NAME+1 );
        name[MAX_NAME+1] = 0;
        status = MINIBASE_DB->add_file_entry( name, status );
        testFailure( status, DBMGR, "Adding a file entry with too long a name" );
      }


    if ( status == OK )
      {
        std::cout << "  - Try to allocate a run of pages that's too long\n";
        status = MINIBASE_DB->allocate_page( pgid, 90 );
        testFailure( status, DBMGR, "Allocating a run that's too long" );
      }

//<<<<<<< db_driver.C
//#ifdef MULTIUSER
//=======

#ifdef    MULTIUSER
//>>>>>>> 1.10
    status = MINIBASE_XACTMGR->commit();
    if( status != OK) {
	std::cerr << "Failure to commit the transaction\n" << std::endl;
    }
#endif

    delete minibase_globals;

    if ( status == OK )
        std::cout << "  Test 3 completed successfully.\n";

    return (status == OK);
}



int DBDriver::test4()
{
    std::cout << "\n  Test 4 tests some boundary conditions.\n"
        "    (These tests are very implementation-specific.)\n";

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
	sprintf(remove_dbcmd, "/bin/rm -rf %s-%s", dbpath, pwd->pw_name);
	sprintf(remove_logcmd, "/bin/rm -rf %s-%s", logpath, pwd->pw_name);
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

    fprintf(stderr,"newdbpath %s,newlogpath %s\n", newdbpath, newlogpath);
    fflush(stderr);

    unlink( newdbpath );
    unlink( newlogpath );

      // We create a new database that's big enough to require 2 pages to hold
      // its space map.
    const int bits_per_page = MAX_SPACE * 8;
    const int dbsize = bits_per_page + 1;
    Status status;
    minibase_globals = new SystemDefs( status, dbpath, logpath, dbsize, 500 );

    PageId pgid = 0;


    std::cout << "  - Make sure no pages are pinned\n";
    if ( MINIBASE_BM->getNumUnpinnedBuffers() != MINIBASE_BM->getNumBuffers() )
      {
        std::cerr << "*** The disk space manager has left pages pinned\n";
        status = FAIL;
      }


    if ( status == OK )
      {
        std::cout << "  - Allocate all pages remaining after DB overhead is accounted for\n";
        status = MINIBASE_DB->allocate_page( pgid, dbsize - 3 );
        if ( status != OK )
            std::cerr << "*** Too little space available: could not allocate " << dbsize-3 << " pages\n";
        else if ( pgid != 3 )
          {
            std::cerr << "*** Expected the first page allocated to be page 3\n";
            status = FAIL;
          }
        else if ( MINIBASE_BM->getNumUnpinnedBuffers() != MINIBASE_BM->getNumBuffers() )
          {
            std::cerr << "*** The disk space manager has left pages pinned\n";
            status = FAIL;
          }
        else
          {
            std::cout << "  - Attempt to allocate one more page\n";
            status = MINIBASE_DB->allocate_page( pgid );
            testFailure( status, DBMGR, "Allocating one additional page" );
          }
      }


    if ( status == OK )
      {
        std::cout << "  - Free some of the allocated pages\n";
        status = MINIBASE_DB->deallocate_page( 3, 7 );
        if ( status == OK )
            status = MINIBASE_DB->deallocate_page( 33, 8 );
        if ( status != OK )
            std::cerr << "*** Error deallocating pages\n";
      }


    if ( status == OK )
      {
        std::cout << "  - Allocate some of the just-freed pages\n";
        status = MINIBASE_DB->allocate_page( pgid, 8 );
        if ( status != OK )
            std::cerr << "*** Could not allocate pages\n";
        else if ( pgid != 33 )
          {
            std::cerr << "*** Allocated wrong run of pages\n";
            status = FAIL;
          }
      }


    if ( status == OK )
      {
        std::cout << "  - Add enough file entries that the directory must surpass a page\n";
        const int count = MAX_SPACE / MAX_NAME + 1; // This over-counts, but uses only public info.
        for ( unsigned i=0; i < count && status == OK; ++i )
          {
            char name[20];
            sprintf( name, "file %u", i );
              // We set every file's first page to be page 0, which doesn't
              // cause an error.
            status = MINIBASE_DB->add_file_entry( name, 0 );
            if ( status != OK )
                std::cerr << "*** Could not add file " << i << std::endl;
          }
      }

    
    if ( status == OK )
      {
        std::cout << "  - Make sure that the directory has taken up an extra page: try to\n"
            "    allocate more pages than should be available\n";
        status = MINIBASE_DB->allocate_page( pgid, 7 );         // There should only be 6 pages available.
        testFailure( status, DBMGR, "Allocating more pages than are now available" );
        if ( status == OK )
          {
            status = MINIBASE_DB->allocate_page( pgid, 6 );     // Should work.
            if ( status != OK )
                std::cerr << "*** But allocating the number that should be available failed.\n";
          }
      }


    if ( status == OK )
      {
        std::cout << "  - At this point, all pages should be claimed.  Try to allocate one more.\n";
        status = MINIBASE_DB->allocate_page( pgid );
        testFailure( status, DBMGR, "Allocating one more page than there is" );
      }


    if ( status == OK )
      {
        std::cout << "  - Free the last two pages: this tests a boundary condition in the space map.\n";
        status = MINIBASE_DB->deallocate_page( dbsize-2, 2 );
        if ( status != OK )
            std::cerr << "*** Did not work.\n";
        else if ( MINIBASE_BM->getNumUnpinnedBuffers() != MINIBASE_BM->getNumBuffers() )
          {
            std::cerr << "*** The disk space manager has left pages pinned\n";
            status = FAIL;
          }
      }
//<<<<<<< db_driver.C
#ifdef MULTIUSER
//=======

#ifdef    MULTIUSER
//>>>>>>> 1.10
    status = MINIBASE_XACTMGR->commit();
    if( status != OK) {
	std::cerr << "Failure to commit the transaction\n" << std::endl;
    }
//<<<<<<< db_driver.C
#endif
//=======
#endif

//>>>>>>> 1.11
    delete minibase_globals;

    if ( status == OK )
        std::cout << "  Test 4 completed successfully.\n";

    delete newdbpath; delete newlogpath;

    return (status == OK);
}

int DBDriver::test5()
{
    return true;
}

int DBDriver::test6()
{
    return true;
}


const char* DBDriver::testName()
{
    return "Disk Space Management";
}

