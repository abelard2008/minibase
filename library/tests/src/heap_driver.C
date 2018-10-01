//*****************************************
//  Driver program for heapfiles
//****************************************

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ostream>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "db.h"
#include "heapfile.h"
#include "scan.h"
#include "heap_driver.h"
#include "buf.h"


static const int namelen = 24;
struct Rec
{
    int ival;
    float fval;
    char name[namelen];
};

static const int reclen = sizeof(Rec);



HeapDriver::HeapDriver() : TestDriver( "hftest" )
{
    choice = 100;       // big enough for file to occupy >1 page
}

HeapDriver::~HeapDriver()
{}


Status HeapDriver::runTests()
{
    return TestDriver::runTests();
}

Status HeapDriver::runAllTests()
{
    Status answer;
    minibase_globals = new SystemDefs(answer,dbpath,logpath,
				      100,500,100,"Clock");
    if ( answer == OK )
        answer = TestDriver::runAllTests();


    delete minibase_globals;
    return answer;
}

const char* HeapDriver::testName()
{
    return "Heap File";
}


//********************************************

int HeapDriver::test1()
{
    std::cout << "\n  Test 1: Insert and scan fixed-size records\n";
    Status status = OK;
    RID rid;

    std::cout << "  - Create a heap file\n";
    HeapFile f("file_1", status);

    if (status != OK)
        std::cerr << "*** Could not create heap file\n";
    else if ( MINIBASE_BM->getNumUnpinnedBuffers() != MINIBASE_BM->getNumBuffers() )
      {
        std::cerr << "*** The heap file has left pages pinned\n";
        status = FAIL;
      }


    if ( status == OK )
      {
        std::cout << "  - Add " << choice << " records to the file\n";
        for (int i =0; i<choice && status == OK; i++)
          {
            Rec rec = { i, i*2.5 };
            sprintf(rec.name, "record %i",i);

            status = f.insertRecord((char *)&rec, reclen, rid);

            if (status != OK)
                std::cerr << "*** Error inserting record " << i << std::endl;
            else if ( MINIBASE_BM->getNumUnpinnedBuffers() != MINIBASE_BM->getNumBuffers() )
              {
                std::cerr << "*** Insertion left a page pinned\n";
                status = FAIL;
              }
          }

        if ( f.getRecCnt() != choice )
          {
            status = FAIL;
            std::cerr << "*** File reports " << f.getRecCnt() << " records, not "
                 << choice << std::endl;
          }
      }


      // In general, a sequential scan won't be in the same order as the
      // insertions.  However, we're inserting fixed-length records here, and
      // in this case the scan must return the insertion order.
    Scan* scan = 0;
    if ( status == OK )
      {
        std::cout << "  - Scan the records just inserted\n";
        scan = f.openScan(status);

        if (status != OK)
            std::cerr << "*** Error opening scan\n";
        else if ( MINIBASE_BM->getNumUnpinnedBuffers() == MINIBASE_BM->getNumBuffers() )
          {
            std::cerr << "*** The heap-file scan has not pinned the first page\n";
            status = FAIL;
          }
      }
    if ( status == OK )
      {
        int len, i = 0;
        Rec rec;

        while ( (status = scan->getNext(rid, (char *)&rec, len)) == OK )
          {
            if ( len != reclen )
              {
                std::cerr << "*** Record " << i << " had unexpected length " << len
                     << std::endl;
                status = FAIL;
                break;
              }
            else if ( MINIBASE_BM->getNumUnpinnedBuffers() == MINIBASE_BM->getNumBuffers() )
              {
                std::cerr << "*** The heap-file scan has not left its page pinned\n";
                status = FAIL;
                break;
              }
            char name[ sizeof rec.name ];
            sprintf( name, "record %i", i );
            if( rec.ival != i  ||
                rec.fval != i*2.5  ||
                0 != strcmp( rec.name, name ) )
              {
                std::cerr << "*** Record " << i << " differs from what we inserted\n";
                status = FAIL;
                break;
              }
            ++i;
          }

        if ( status == DONE )
          {
            if ( MINIBASE_BM->getNumUnpinnedBuffers() != MINIBASE_BM->getNumBuffers() )
                std::cerr << "*** The heap-file scan has not unpinned its page after finishing\n";
            else if ( i == choice )
                status = OK;
            else
                std::cerr << "*** Scanned " << i << " records instead of "
                     << choice << std::endl;
          }
      }

    delete scan;

    if ( status == OK )
        std::cout << "  Test 1 completed successfully.\n";
    return (status == OK);
}


//***************************************************

int HeapDriver::test2()
{
    std::cout << "\n  Test 2: Delete fixed-size records\n";
    Status status = OK;
    Scan* scan = 0;
    RID rid;

    std::cout << "  - Open the same heap file as test 1\n";
    HeapFile f("file_1", status);

    if (status != OK)
        std::cerr << "*** Error opening heap file\n";


    if ( status == OK )
      {
        std::cout << "  - Delete half the records\n";
        scan = f.openScan(status);
        if (status != OK)
            std::cerr << "*** Error opening scan\n";
      }
    if ( status == OK )
      {
        int len, i = 0;
        Rec rec;

        while ( (status = scan->getNext(rid, (char *)&rec, len)) == OK )
          {
            if ( i & 1 )        // Delete the odd-numbered ones.
              {
                status = f.deleteRecord( rid );
                if ( status != OK )
                  {
                    std::cerr << "*** Error deleting record " << i << std::endl;
                    break;
                  }
              }
            ++i;
          }

        if ( status == DONE )
            status = OK;
      }

    delete scan;
    scan = 0;

    if ( status == OK
         && MINIBASE_BM->getNumUnpinnedBuffers() != MINIBASE_BM->getNumBuffers() )
      {
        std::cerr << "*** Deletion left a page pinned\n";
        status = FAIL;
      }

    if ( status == OK )
      {
        std::cout << "  - Scan the remaining records\n";
        scan = f.openScan(status);
        if (status != OK)
            std::cerr << "*** Error opening scan\n";
      }
    if ( status == OK )
      {
        int len, i = 0;
        Rec rec;

        while ( (status = scan->getNext(rid, (char *)&rec, len)) == OK )
          {
            if( rec.ival != i  ||
                rec.fval != i*2.5 )
              {
                std::cerr << "*** Record " << i << " differs from what we "
                    "inserted\n";
                status = FAIL;
                break;
              }
            i += 2;     // Because we deleted the odd ones...
          }

        if ( status == DONE )
            status = OK;
      }

    delete scan;

    if ( status == OK )
        std::cout << "  Test 2 completed successfully.\n";
    return (status == OK);
}


//********************************************************

int HeapDriver::test3()
{
    std::cout << "\n  Test 3: Update fixed-size records\n";
    Status status = OK;
    Scan* scan = 0;
    RID rid;

    std::cout << "  - Open the same heap file as tests 1 and 2\n";
    HeapFile f("file_1", status);

    if (status != OK)
        std::cerr << "*** Error opening heap file\n";


    if ( status == OK )
      {
        std::cout << "  - Change the records\n";
        scan = f.openScan(status);
        if (status != OK)
            std::cerr << "*** Error opening scan\n";
      }
    if ( status == OK )
      {
        int len, i = 0;
        Rec rec;

        while ( (status = scan->getNext(rid, (char *)&rec, len)) == OK )
          {
            rec.fval = 7*i;     // We'll check that i==rec.ival below.
            status = f.updateRecord( rid, (char*)&rec, len );
            if ( status != OK )
              {
                std::cerr << "*** Error updating record " << i << std::endl;
                break;
              }
            i += 2;     // Recall, we deleted every other record above.
          }

        if ( status == DONE )
            status = OK;
      }

    delete scan;
    scan = 0;

    if ( status == OK
         && MINIBASE_BM->getNumUnpinnedBuffers() != MINIBASE_BM->getNumBuffers() )
      {
        std::cerr << "*** Updating left pages pinned\n";
        status = FAIL;
      }

    if ( status == OK )
      {
        std::cout << "  - Check that the updates are really there\n";
        scan = f.openScan(status);
        if (status != OK)
            std::cerr << "*** Error opening scan\n";
      }
    if ( status == OK )
      {
        int len, i = 0;
        Rec rec, rec2;

        while ( (status = scan->getNext(rid, (char *)&rec, len)) == OK )
          {
              // While we're at it, test the getRecord method too.
            status = f.getRecord( rid, (char*)&rec2, len );
            if ( status != OK )
              {
                std::cerr << "*** Error getting record " << i << std::endl;
                break;
              }
            if( rec.ival != i || rec.fval != i*7
                || rec2.ival != i || rec2.fval != i*7 )
              {
                std::cerr << "*** Record " << i << " differs from our "
                    "update\n";
                status = FAIL;
                break;
              }
            i += 2;     // Because we deleted the odd ones...
          }

        if ( status == DONE )
            status = OK;
      }

    delete scan;

    if ( status == OK )
        std::cout << "  Test 3 completed successfully.\n";
    return (status == OK);
}


//*****************************************************************

int HeapDriver::test4()
{
    std::cout << "\n  Test 4: Temporary heap files and variable-length records\n";
    Status status = OK;
    Scan* scan = 0;
    RID rid;

    std::cout << "  - Create a temporary heap file\n";
    HeapFile f( 0, status );

    if (status != OK)
        std::cerr << "*** Error creating temp file\n";


    if ( status == OK )
        std::cout << "  - Add variable-sized records\n";

    unsigned recSize = MINIBASE_PAGESIZE / 2;
      /* We use half a page as the starting size so that a single page won't
         hold more than one record.  We add a series of records at this size,
         then halve the size and add some more, and so on.  We store the index
         number of each record on the record itself. */
    unsigned numRecs = 0;
    char record[MINIBASE_PAGESIZE] = "";
    for ( ; recSize >= (sizeof numRecs + sizeof recSize) && status == OK;
          recSize /= 2 )
        for ( unsigned i=0; i < 10 && status == OK; ++i, ++numRecs )
          {
            memcpy( record, &numRecs, sizeof numRecs );
            memcpy( record+(sizeof numRecs), &recSize, sizeof recSize );
            status = f.insertRecord( record, recSize, rid );
            if ( status != OK )
                std::cerr << "*** Failed inserting record " << numRecs
                     << ", of size " << recSize << std::endl;
          }


    int seen[numRecs];
    memset( seen, 0, sizeof seen );
    if ( status == OK )
      {
        std::cout << "  - Check that all the records were inserted\n";
        scan = f.openScan(status);
        if (status != OK)
            std::cerr << "*** Error opening scan\n";
      }
    if ( status == OK )
      {
        int len;
        while ( (status = scan->getNext(rid, record, len)) == OK )
          {
            unsigned recNum;
            memcpy( &recNum, record, sizeof recNum );
            if ( seen[recNum] )
              {
                std::cerr << "*** Found record " << recNum << " twice!\n";
                status = FAIL;
                break;
              }
            seen[recNum] = TRUE;

            memcpy( &recSize, record + sizeof recNum, sizeof recSize );
            if ( recSize != (unsigned)len )
              {
                std::cerr << "*** Record size mismatch: stored " << recSize
                     << ", but retrieved " << len << std::endl;
                status = FAIL;
                break;
              }

            --numRecs;
          }

        if ( status == DONE )
            if ( numRecs )
                std::cerr << "*** Scan missed " << numRecs << " records\n";
            else
                status = OK;
      }

    delete scan;

    if ( status == OK )
        std::cout << "  Test 4 completed successfully.\n";
    return (status == OK);
}



int HeapDriver::test5()
{
    std::cout << "\n  Test 5: Test some error conditions\n";
    Status status = OK;
    Scan* scan = 0;
    RID rid;

    HeapFile f("file_1", status);
    if (status != OK)
        std::cerr << "*** Error opening heap file\n";

    if ( status == OK )
      {
        std::cout << "  - Try to change the size of a record\n";
        scan = f.openScan(status);
        if (status != OK)
            std::cerr << "*** Error opening scan\n";
      }
    if ( status == OK )
      {
        int len;
        Rec rec;
        status = scan->getNext(rid, (char *)&rec, len);
        if ( status != OK )
            std::cerr << "*** Error reading first record\n";
        else
          {
            status = f.updateRecord( rid, (char*)&rec, len-1 );
            testFailure( status, HEAPFILE, "Shortening a record" );
            if ( status == OK )
              {
                status = f.updateRecord( rid, (char*)&rec, len+1 );
                testFailure( status, HEAPFILE, "Lengthening a record" );
              }
          }
      }

    delete scan;


    if ( status == OK )
      {
        std::cout << "  - Try to insert a record that's too long\n";
        char record[MINIBASE_PAGESIZE] = "";
        status = f.insertRecord( record, MINIBASE_PAGESIZE, rid );
        testFailure( status, HEAPFILE, "Inserting a too-long record" );
      }



    if ( status == OK )
        std::cout << "  Test 5 completed successfully.\n";
    return (status == OK);
}


int HeapDriver::test6()
{
    return TRUE;
}
