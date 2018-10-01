/*
    bmtrace

    Usage:  bmtrace [option] test_number 

    options:
      -n x        set number of frames in buffer to x
      -r x        set replacement strategy to x (x can be lru, mru, or clk)
 */


#include <stdio.h>
#include <ostream>
#include <string.h>
#include <stdlib.h>
#include "buf.h"
#include "lru.h"
#include "mru.h"

static unsigned Num_frames = 64;
enum { lru, mru, clk, rdm } Strategy = lru;

static void singleScan();
static void scanWithWrites();
static void severalScans();
static void scansWithOtherReads();
static void pageNestedLoops();
static void blockNestedLoops1();
static void blockNestedLoops2();
static void clusteredScan();
static void unclusteredScan();
static void indexNestedLoops();
//static void externalSort();

static struct {
    void (*fcn)();
    const char* desc;
} tests[] = {
    { singleScan,             "Single linear scan of a file" },
    { scanWithWrites,         "Linear scan with occasional writes" },
    { severalScans,           "Several consecutive scans of a file" },
    { scansWithOtherReads,    "Consecutive scans with occasional reads of other file" },
    { pageNestedLoops,        "Simple page nested loops" },
    { blockNestedLoops1,      "Block nested loops join, strategy 1" },
    { blockNestedLoops2,      "Block nested loops join, strategy 2" },
    { clusteredScan,          "Clustered index scan" },
    { unclusteredScan,        "Unclustered index scan" },
    { indexNestedLoops,       "Index nested loops join" },
//    { externalSort,           "External sort of a file" },
};

static const num_tests = sizeof tests / sizeof tests[0];


void usage()
{
    std::cerr << "Usage: bmtrace [-n num_frames] [-r replacer] test_number" << endl;
    std::cerr << "       num_frames in 16..64" << endl;
    std::cerr << "       replacer = lru | mru | clk" << endl;
    std::cerr << endl;
    std::cerr << "       default num_frames = 64" << endl;
    std::cerr << "       default replacer = lru" << endl;
    std::cerr << endl;
    for ( int tn=0; tn < num_tests; ++tn )
      std::cerr << "  Test " << tn+1 << ": " << tests[tn].desc << endl;
    exit(1);
}


char** parse(int argc, char** argv)
{
    while (*++argv)
      {
        if ( **argv != '-' )
          {
            if ( argv[1] )
              usage();
            return argv;
          }
        if ( 0 == strcmp(*argv, "-n") )
          {
            ++argv;
            if ( !*argv || sscanf(*argv, "%u", &Num_frames) != 1 ||
                 Num_frames > 64 || Num_frames < 16 )
                usage();
          }
        else if ( 0 == strcmp(*argv, "-r"))
          {
            ++argv;
            if (!*argv)
                usage();
            if ( 0 == strcmp(*argv, "lru") )
                Strategy = lru;
            else if ( 0 == strcmp(*argv, "mru") )
                Strategy = mru;
            else if ( 0 == strcmp(*argv, "clk") )
                Strategy = clk;
            else if ( 0 == strcmp(*argv, "rdm") )
              { std::cerr << "Random replacer not yet implemented\n"; usage(); }
            else
                usage();
          }
      }

    usage();
    return 0;                                   // Will never get here.
}


DB* db;    
BufMgr* bm;   
global_error* minirel_error;

SpaceMgr* space_mgr;
lock_manager_def* lockMgr;


#define CHK(STATUS) do { if(STATUS!=OK){ minirel_error->show_errors();exit(1); } }while(0)


void startup()
{
    std::cerr << "------------- Buffer Manager Starting Up --------------" << endl;

      // Create db and bm.
    Status status;
    minirel_error = new global_error;
    minirel_error->registerErrorMsgs((char***)&bufErrMsgs,BUFMGR);
    minirel_error->registerErrorMsgs((char***)&dbErrMsgs,DBMGR);
    space_mgr = new SpaceMgr;

      // Start the BM trace.
    BufMgr::Trace = &std::cout;

    Replacer* replacer = 0;

    switch ( Strategy )
      {
        case lru: replacer = new LRU; break;
        case mru: replacer = new MRU; break;
        case clk: replacer = new Clock; break;
        case rdm: /*replacer = new RandomReplacer;*/ break;
      }

    bm = new BufMgr( Num_frames, replacer );


    db = new DB( "bmtrace.minibase", 200, status );
    CHK(status);
}


void shutdown()
{
    std::cerr << "------------- Buffer Manager Shutting Down --------------" << endl;

    db->db_release();
    db->DeleteDSM();
    delete bm; // Note order: it breaks otherwise.
    delete db;
    delete space_mgr;
    delete minirel_error;
}



main( int argc, char **argv )
{
    int test_number = atoi( *parse(argc, argv) );
    if ( test_number < 1 || test_number > num_tests )
        usage();

    startup();
    std::cerr << "--------- Running test " << test_number << ": " << tests[test_number-1].desc << endl;
    tests[test_number-1].fcn();
    shutdown();

    return 0;
}


static void singleScan()
/*
    Do linear scan on a file.
 */
{
    Page* pg;

    for ( PageId i = 10; i < 50; ++i )
      {
        CHK( bm->pinPage( i, pg, FALSE, "test-file" ) );
        CHK( bm->unpinPage( i, FALSE, "test-file" ) );
      }
}


static void scanWithWrites()
/*
    Do one linear scan on a file, with occasional set_dirty.
 */
{
    Page* pg;

    for ( PageId i = 10; i < 50; ++i )
      {
        CHK( bm->pinPage( i, pg, FALSE, "test-file" ) );
        CHK( bm->unpinPage( i, (i % 10) == 7, "test-file" ) );
      }
}



static void severalScans()
/*
    Do multiple linear scans on a file.
 */
{
    Page* pg;

    for ( int j = 0; j < 4; ++j )
      for ( PageId i = 10; i < 30; ++i )
        {
          CHK( bm->pinPage( i, pg, FALSE, "test-file" ) );
          CHK( bm->unpinPage( i, FALSE, "test-file" ) );
        }
}


static void scansWithOtherReads()
/*
    Do multiple scans with occasional read from another file.
 */
{
    Page* pg;

    for ( int j = 0; j < 4; ++j )
      for ( PageId i = 10; i < 30; ++i )
        {
          CHK( bm->pinPage( i, pg, FALSE, "file1" ) );

          if ( rand() % 10 == 3 )
            {
              PageId i2 = 60 + rand() % 10;
              CHK( bm->pinPage( i2, pg, FALSE, "file2" ) );
              CHK( bm->unpinPage( i2, FALSE, "file2" ) );
            }

          CHK( bm->unpinPage( i, FALSE, "file1" ) );
        }
}


static void pageNestedLoops()
/*
    Do nested loop scan.
 */
{
    Page* pg;

      // Our two files are: 1: pages 10-19; 2: pages 60-79.

      // This test walks the two files to simulate a simple
      // page-oriented nested loop scan.

    for ( PageId i = 10; i < 20; ++i )
      {
        CHK( bm->pinPage( i, pg, FALSE, "file1" ) );

        for ( PageId j = 60; j < 80; ++j )
          {
            CHK( bm->pinPage( j, pg, FALSE, "file2" ) );
            CHK( bm->unpinPage( j, FALSE, "file2" ) );
          }

        CHK( bm->unpinPage( i, FALSE, "file1" ) );
      }
}


static inline int min( int a, int b ) { return a < b? a : b; }

static void blockNestedLoops1()
/*
    Simulate nested loop join.  Strategy 1: read as much of file 1 as possible.
 */
{
    Page *file1Page, *file2Page, *outputPage;

      // Our two input files are: 1: pages 10-49; 2: pages 50-64.
      // Our output file is pages 70-99.

      // This test walks the two files to simulate a block-oriented
      // nested loop join.  Our strategy is to read as much of file 1
      // as possible, then cycle through file 2 one page at a time,
      // then read more of file 1 and repeat the process.


      // We subtract 1 for a page of file 2 and 1 more for a page of
      // the output file.
    unsigned maxFile1Pinned = bm->getNumUnpinnedBuffers() - 2;


    unsigned size1 = 40, size2 = 15;    // Sizes of files 1 and 2.
    PageId file1Start = 10, file2Start = 50, outputPageId = 70;

      // We assume that two output pages are accumulated for each page
      // of input file 2.
    unsigned pagesPerOutputPage = size1 / 2;
    unsigned numPagesJoined = 0;



      // Walk through file 1 a block at a time.
    for ( int i = 0; i < size1; i += maxFile1Pinned )
      {
        unsigned blockSize = min( maxFile1Pinned, size1 - i );

          // First pin the entire block.
        for ( int j = 0; j < blockSize; ++j )
          CHK( bm->pinPage( i + j + file1Start, file1Page, FALSE, "file1" ) );

          // Now walk through all of file 2.
        for ( int i2 = 0; i2 < size2; ++i2 )
          {
            CHK( bm->pinPage( i2 + file2Start, file2Page, FALSE, "file2" ) );

              // Now compare each pinned page from file 1 against the
              // current page of file 2.
            for ( j = 0; j < blockSize; ++j )
              {
                  // This is where the join takes place.  We
                  // accumulate data from file1Page and file2Page and
                  // write it to outputPage.
                if ( numPagesJoined == 0 )    // The TRUE below means don't read it first.
                  CHK( bm->pinPage( outputPageId, outputPage, TRUE, "output" ) );


                  // When we have accumulated enough data on the
                  // output page, we mark it dirty and unpin it, then
                  // go on to the next output page.
                if ( ++numPagesJoined >= pagesPerOutputPage )
                  {
                    numPagesJoined = 0;
                    CHK( bm->unpinPage( outputPageId, TRUE, "output" ) );  // The TRUE marks it dirty.
                    ++outputPageId;
                  }
              }

            CHK( bm->unpinPage( i2 + file2Start, FALSE, "file2" ) );
          }

          // Finally, unpin the entire block of file 1.
        for ( j = 0; j < blockSize; ++j )
          CHK( bm->unpinPage( i + j + file1Start, FALSE, "file1" ) );
      }

      // If there is data accumulated on the output page, write it out.
    if ( numPagesJoined > 0 )
      CHK( bm->unpinPage( outputPageId, TRUE, "output" ) );
}


static void blockNestedLoops2()
/*
    Simulate nested loop join.  Strategy 2: read the same amount of both files.
 */
{
    Page *file1Page, *file2Page, *outputPage;

      // Our two input files are: 1: pages 10-49; 2: pages 50-64.
      // Our output file is pages 70-99.

      /* This test walks the two files to simulate a block-oriented
         nested loop join.  Our strategy is to split the available
         buffer space between blocks of file 1 and file 2.  */


      // We subtract 1 for a page of the output file.
    unsigned availableSpace = bm->getNumUnpinnedBuffers() - 1;


    unsigned size1 = 40, size2 = 15;    // Sizes of files 1 and 2.
    PageId file1Start = 10, file2Start = 50, outputPageId = 70;


    unsigned blockSize1 = availableSpace / 2;
    if ( blockSize1 > size1 ) blockSize1 = size1;
    unsigned blockSize2 = availableSpace - blockSize1;
    if ( blockSize2 > size2 )
      {
        blockSize2 = size2;
        blockSize1 = availableSpace - blockSize2;
      }


      // We assume that two output pages are accumulated for each page
      // of input file 2.
    unsigned pagesPerOutputPage = size1 / 2;
    unsigned numPagesJoined = 0;



      // Walk through file 1 a block at a time.
    for ( int i = 0; i < size1; i += blockSize1 )
      {
        unsigned thisBlockSize1 = min( blockSize1, size1 - i );

          // First pin the entire block from file 1.
        for ( int j = 0; j < thisBlockSize1; ++j )
          CHK( bm->pinPage( i + j + file1Start, file1Page, FALSE, "file1" ) );

          // Walk through file 2 one block at a time.
        for ( int i2 = 0; i2 < size2; i2 += blockSize2 )
          {
            unsigned thisBlockSize2 = min( blockSize2, size2 - i2 );

              // Pin the entire file 2 block.
            for ( int j2 = 0; j2 < thisBlockSize2; ++j2 )
              CHK( bm->pinPage( i2 + j2 + file2Start, file2Page, FALSE, "file2" ) );

              // Now compare each pinned page from file 1 against each
              // pinned page of file 2.
            for ( j = 0; j < thisBlockSize1; ++j )
              for ( j2 = 0; j2 < thisBlockSize2; ++j2 )
                {
                    // This is where the join takes place.  We
                    // accumulate data from file1Page and file2Page and
                    // write it to outputPage.
                  if ( numPagesJoined == 0 )
                    CHK( bm->pinPage( outputPageId, outputPage, TRUE, "output" ) );
                  
                  
                    // When we have accumulated enough data on the
                    // output page, we mark it dirty and unpin it, then
                    // go on to the next output page.
                  if ( ++numPagesJoined >= pagesPerOutputPage )
                    {
                      numPagesJoined = 0;
                      CHK( bm->unpinPage( outputPageId, TRUE, "output" ) ); // The TRUE marks it dirty.
                      ++outputPageId;
                    }
                }
            
              // Unpin the file 2 block.
            for ( j2 = 0; j2 < thisBlockSize2; ++j2 )
              CHK( bm->unpinPage( i2 + j2 + file2Start, FALSE, "file2" ) );
          }
        
        // Finally, unpin the entire block of file 1.
        for ( j = 0; j < thisBlockSize1; ++j )
          CHK( bm->unpinPage( i + j + file1Start, FALSE, "file1" ) );
      }
    
    // If there is data accumulated on the output page, write it out.
    if ( numPagesJoined > 0 )
      CHK( bm->unpinPage( outputPageId, TRUE, "output" ) );
}


static void clusteredScan()
     /*
        Clustered index.
        */
{
    Page* pg;

      // File 1 is the index: pages 10-19.  File 2 is the relation: pages 20-59.

      // This test simulates walking the index and fetching the
      // corresponding tuple for each index entry.

    PageId relationPage = 20;
    unsigned relationPagesPerIndexPage = 4;

    for ( PageId indexPage = 10; indexPage < 20; ++indexPage )
      {
        CHK( bm->pinPage( indexPage, pg, FALSE, "index" ) );

        for ( unsigned rp = 0; rp < relationPagesPerIndexPage; ++rp )
          {
            CHK( bm->pinPage( relationPage, pg, FALSE, "relation" ) );
              // Here, we read the tuples from one of the relation pages
              // that corresponds to the current index page.
            CHK( bm->unpinPage( relationPage, FALSE, "relation" ) );
            ++relationPage;
          }

        CHK( bm->unpinPage( indexPage, FALSE, "index" ) );
      }
}


static void unclusteredScan()
     /*
        Unclustered index.
        */
{
    Page* pg;

      // File 1 is the index: pages 10-19.  File 2 is the relation: pages 20-39.

      // This test simulates walking the index and fetching the
      // corresponding tuple for each index entry.

    PageId firstRelationPage = 20;
    unsigned numRelationPages = 20;
    unsigned entriesPerIndexPage = 10;

    for ( PageId indexPage = 10; indexPage < 20; ++indexPage )
      {
        CHK( bm->pinPage( indexPage, pg, FALSE, "index" ) );

        for ( unsigned entry = 0; entry < entriesPerIndexPage; ++entry )
          {
            PageId relationPage = firstRelationPage + rand() % numRelationPages;
            CHK( bm->pinPage( relationPage, pg, FALSE, "relation" ) );
              // Here, we read the tuple that corresponds to the current index entry.
            CHK( bm->unpinPage( relationPage, FALSE, "relation" ) );
          }

        CHK( bm->unpinPage( indexPage, FALSE, "index" ) );
      }
}


static void indexNestedLoops()
{
    /* We have two relation files: 1: pages 10-19; 2: pages 30-69.  We
       have an index on relation 2; the root page of the index is 70
       and the leaf pages are 71-80.  Our output file starts at page 90.

       This test simulates an index nested loop join.  We walk
       relation 1; for each tuple we look up the corresponding value
       in the index, then fetch the indicated tuples from relation 2
       and write the result to the output file.

     */


    unsigned size1 = 10, size2 = 40;    // Sizes of relations 1 and 2.
    unsigned numIndexLeaves = 10;
    PageId rel1Start = 10, rel2Start = 30, indexRoot = 70, indexStart = 71;
    PageId outputPageId = 90;
    Page *rel1Page, *indexRootPage, *indexLeafPage, *rel2Page, *outputPage;

      // How many tuples are on each page of relation 1?
    unsigned tuplesPerPage1 = 6;


      // We assume that each output page holds 15 joined (and projected) tuples.
    unsigned tuplesPerOutputPage = 15;
    unsigned numTuplesJoined = 0;


      // We pin the root page of the index once...
    CHK( bm->pinPage( indexRoot, indexRootPage, FALSE, "index" ) );


      // Walk through relation 1 a page at a time.
    for ( int i1 = 0; i1 < size1; ++i1 )
      {
        CHK( bm->pinPage( rel1Start + i1, rel1Page, FALSE, "relation1" ) );

          // Walk through the tuples of this page.
        for ( int j = 0; j < tuplesPerPage1; ++j )
          {
              // Look on the index root page to find the appropriate index leaf
              // page, then look there to find the corresponding pages of
              // relation 2.
            PageId indexLeaf = indexStart + rand() % numIndexLeaves;
            CHK( bm->pinPage( indexLeaf, indexLeafPage, FALSE, "index" ) );

            unsigned numMatchingTuples = rand() % 4;
            for ( int i2 = 0; i2 < numMatchingTuples; ++i2 )
              {
                PageId rel2PageId = rel2Start + rand() % size2;
                CHK( bm->pinPage( rel2PageId, rel2Page, FALSE, "relation2" ) );

                  // This is where the join takes place.  We accumulate data
                  // from rel1Page and rel2Page and write it to outputPage.
                if ( numTuplesJoined == 0 )
                    CHK( bm->pinPage( outputPageId, outputPage, TRUE, "output" ) );
                  
                  // When we have accumulated enough data on the output page,
                  // we mark it dirty and unpin it, then go on to the next
                  // output page.
                if ( ++numTuplesJoined >= tuplesPerOutputPage )
                  {
                    numTuplesJoined = 0;
                    CHK( bm->unpinPage( outputPageId, TRUE, "output" ) );
                    ++outputPageId;
                  }

                CHK( bm->unpinPage( rel2PageId, FALSE, "relation2" ) );
              }
            
              // Now unpin the index leaf page.
            CHK( bm->unpinPage( indexLeaf, FALSE, "index" ) );
          }

        CHK( bm->unpinPage( rel1Start + i1, FALSE, "relation1" ) );
      }
    
      // If there is data accumulated on the output page, write it out.
    if ( numTuplesJoined > 0 )
        CHK( bm->unpinPage( outputPageId, TRUE, "output" ) );


      // Finally, unpin the root page of the index.
    CHK( bm->unpinPage( indexRoot, FALSE, "index" ) );
}


#if 0				// Not done yet!
static void externalSort()
     /*
        Sort a file, leave the result in a temporary file.
      */
{
      /* We simulate the I/O pattern of external sorting.  We read the input
         file, and sort as many pages of it as will fit at once into the buffer
         pool.  We put the resulting sorted runs into a temporary file, then go
         back through it and merge the sorted runs into longer sorted runs,
         which we put into a second temporary file.  We then ping-pong between
         the two temporary files until we end up with a single run. */

      // Our input file has 40 pages, and therefore so do both of the temporaries.
    PageId fileStart = 20, temp1Start = 60, temp2Start = 100;
    unsigned fileSize = 40;
    unsigned blockSize = bm->getNumUnpinnedBuffers() - 1;



      // To simulate multi-pass sorting, we need to allocate a bunch of buffers
      // to other processes.
    Page *pg1, *pg2;
    for ( int i = 10; i < 20; ++i )
        CHK( bm->pinPage( i, pg1, FALSE, "other-process" ) );



      // Now we do the initial sort of the input file.
    for ( i = 0; i < fileSize; i+=blockSize )
      {
        unsigned thisBlockSize = min( blockSize1, fileSize - i );
        for ( int j = 0; j < thisBlockSize; ++j )
          {
            CHK( bm->pinPage( fileStart + i + j, pg2, FALSE, "file" ) );
            CHK( bm->pinPage( temp1Start + i + j, pg1, TRUE, "temp1" ) );
              // Here, copy pg2 to pg1.
            CHK( bm->unpinPage( fileStart + i + j, FALSE, "file" ) );
          }

          // Here we would sort the pinned pages from temp1.  When done, write
          // them out.
        for ( j = 0; j < thisBlockSize; ++j )
            CHK( bm->unpinPage( temp1Start + i + j, TRUE, "temp1" ) );
      }



      // Now we loop, merging runs until our run length equals the size of the
      // file.
    int inTemp1 = TRUE;
    while ( blockSize < fileSize )
      {
        const char* inName = inTemp1? "temp1" : "temp2";
        const char* outName = inTemp1? "temp2" : "temp1";
        unsigned inStart = inTemp1? temp1Start : temp2Start;
        unsigned outStart = inTemp1? temp2Start : temp1Start;

        unsigned newBlockSize = blockSize * blockSize;

        for ( i = 0; i < fileSize; i += newBlockSize )
            for ( int j = 0; j < blockSize; ++j )
                for ( int k = 0; k < blockSize; ++k )
                  {
                    unsigned inPage = i + (j * blockSize) + k;
                    if ( inPage < fileSize )
                      {
                        if ( j )
                            CHK( bm->unpinPage( inStart + inPage - 1, FALSE, inName ) );
                        CHK( bm->pinPage( inStart + inPage, pg1, FALSE, inName ) );
                      }
                  }

        inTemp1 = !inTemp1;
        blockSize = newBlockSize;
      }



      // At the end of the day, release the "other process's" pages too.
    for ( i = 10; i < 20; ++i )
        CHK( bm->unpinPage( i, FALSE, "other-process" ) );
}
#endif
