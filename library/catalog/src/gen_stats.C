/* ---------------------------------------------------------- */

// 
// Written by Amit Shukla while TA for cs564 (11/2/95)
// This generates statistics for the tables in the database
// The interation is with the catalog.
//
// The following information if generated:
// 
//   1. data for a relation
//      o # of tuples in the relation
//      o # of pages in the heapfile (available from hf hdr page).
//
//   2. nested inside each relation, each attribute
//      o Max key, min key values
//
//   3. nested inside each attribute any relevant accessmethods
//      o # of distinct key values
//      o # of index leaf pages.
// 


#include "maincatalog.h"
#include "catalog.h"
#include "file_scan.h"
#include "dupl_elim.h"
extern "C" int strcasecmp( const char*, const char* );

/* ---------------------------------------------------------- */

void test_and_set(char   min[], char   max[], Tuple * tuple, int colNo);
void test_and_set(int  & min,   int  & max,   Tuple * tuple, int colNo);
void test_and_set(float& min,   float& max,   Tuple * tuple, int colNo);

/* ---------------------------------------------------------- */


Status RelCatalog::runStats(const std::string filename)      
{
   RelDesc relRec;
   AttrDesc *attrRecs = NULL;
   std::fstream outFile;
   Scan *pscan = NULL;
   int relCount = 0 ;
   int recSize;

   RID rid;

   int indexPlan = 0;
   Status status;
 
   pscan = openScan(status);
   if(! pscan)
       return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );

   // OPEN BOGUSCCATALOGS

   outFile.open(filename, std::ios::out);
   // CHECK FOR BAD OPEN HERE!!!!!!!!!!!!!11
   outFile << std::endl << std::endl ;


   // SCAN THE RELATION CATALOG (A HEAPFILE)

   while (1)
   {
      // get each relation
      status = pscan->getNext(rid, (char *) tuple, recSize);

      if (status == DONE)
      {
	 // output number of relations
        outFile.seekg(0,std::ios::beg);
	 outFile << relCount ;
	 status = OK;
	 outFile.close();
	 break;
      }
      else if (status == OK)
      {
	 assert(tuple->size() == recSize);

	 // Decode the tuple into the relRec structure for easier access.
	 read_tuple(tuple, relRec);

	 // get all attributes for the relation
	 status = MINIBASE_ATTRCAT->getRelInfo(relRec.relName, relRec.attrCnt, attrRecs);

	 if (status != OK) break;

	 // Compute tuple size (needed by optimizer).
         int tupleSize = 0;
	 for (int i = 0 ; i < relRec.attrCnt; i++)
	    tupleSize = tupleSize + attrRecs[i].attrLen;

	 genStats(relRec, attrRecs);

	 //  *** dump relation record to disk ***
	 // Now determine information for the relation.
	 status = dumpRelation(outFile, relRec, tupleSize);

	 if (status != OK) break;

	 // DUMP ATTRIBUTES TO DISK -> ACCESSMETHODS WILL ALSO BE DUMPED
	 // Determine attribute level statistics. For each attribute also
	 // determine index level stats.
	 status = rsdumpRelAttributes(outFile, attrRecs, relRec.attrCnt,
				      relRec.relName);

	 if(status != OK) break;

	 // NOW DUMP A FILE SCAN PLAN AFTER THE LAST ATTRIBUTE
	 outFile << "FileScan" << " " ;
	 outFile << relRec.numPages << " "; 
	 outFile << "R" << " ";
	 outFile << indexPlan << std::endl; 

	 relCount++; 
	 delete [] attrRecs;   // MUST DELETE ON EACH ITERATION
	 attrRecs = NULL;
      }
      else 
	 break;
   }
   if(attrRecs)
      delete [] attrRecs;
   delete pscan;
   outFile.close();
   if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

   return OK ;
}


/* ---------------------------------------------------------- */

//
// Skip generating statistics for the system catalogs
//

Status
RelCatalog::genStats(RelDesc &relRec, AttrDesc *&attrRecs)
{
   // Do not generate statistics for the catalog tables...
   if (strcasecmp(relRec.relName,   RELCATNAME) == 0 || 
       strcasecmp(relRec.relName,  ATTRCATNAME) == 0 || 
       strcasecmp(relRec.relName, INDEXCATNAME) == 0)
      return OK;				// Don't generate stats

   Status status = OK;
   int    cnt = 0;
   RID    rid;
   int    recSize;
   Tuple *tuple = (Tuple *) new char [Tuple::max_size];

   // determine the number of tuples.
   HeapFile *hf = new HeapFile(relRec.relName, status);
   Scan     *ps = NULL;
   char      ms[2];

   // Initialize the min and max values...
   for(int i = 0; i < relRec.attrCnt; i++)
   {
      switch (attrRecs[i].attrType)
      {
       case attrString  :
	 ms[1] = '\0';
	 ms[0] = char(127);
	 strcpy(attrRecs[i].minVal.strVal, ms);
	 ms[0] = char(0x1);
	 strcpy(attrRecs[i].maxVal.strVal, ms);
	 break;
       case attrReal    :
	 attrRecs[i].minVal.floatVal =  1.0e9;
	 attrRecs[i].maxVal.floatVal = -1.0e9;
	 break;
       case attrInteger :
	 attrRecs[i].minVal.intVal   =  999999999;
	 attrRecs[i].maxVal.intVal   = -999999999;
	 break;
       default:
	 break;
      }
   }

   ps = hf->openScan(status);
   while ((status = ps->getNext(rid, (char *) tuple, recSize)) != DONE)
   {
      // Check the min, max key values for each attribute.
      for (int i = 0; i < relRec.attrCnt; i++)
      {
	 // If the value in the tuple is greater than the 
	 switch (attrRecs[i].attrType)
	 {
	  case attrString  :
	    test_and_set(attrRecs[i].minVal.strVal, attrRecs[i].maxVal.strVal,
			 tuple, i+1);
	    break;
	  case attrReal    :
	    test_and_set(attrRecs[i].minVal.floatVal,
			 attrRecs[i].maxVal.floatVal, tuple, i+1);
	    break;
	  case attrInteger :
	    test_and_set(attrRecs[i].minVal.intVal, attrRecs[i].maxVal.intVal,
			 tuple, i+1);
	    break;
	  default:
	    break;
	 }
      }
      cnt++;
   }

   relRec.numTuples = cnt;
   relRec.numPages  = 10; // TO DO hf->n_pages();
   
   delete ps;
   delete hf;


   delete [] (char *) tuple;
   return OK;
}


void test_and_set(char min[], char max[], Tuple * tuple, int colNo)
{
   const int arr_sz = 9;

   if (strcmp(min, tuple->getFld(colNo)) > 0)
      strncpy(min, tuple->getFld(colNo), arr_sz);

   if (strcmp(max, tuple->getFld(colNo)) < 0)
      strncpy(max, tuple->getFld(colNo), arr_sz);

   // Now, another Kludge: Search for a ' ' and set it to '\0'
   for (int i = 0; i <= arr_sz; i++)
   {
      if (min[i] == ' ')
      {
	 min[i] = '\0';
	 break;
      }
   }

   for (int i = 0; i <= arr_sz; i++)
   {
      if (max[i] == ' ')
      {
	 max[i] = '\0';
	 break;
      }
   }
}


void test_and_set(int & min, int & max, Tuple * tuple, int colNo)
{
   int temp_i;

   temp_i = tuple->getFld(colNo, temp_i);
   if (min > temp_i)
      min = temp_i;

   if (max < temp_i)
      max = temp_i;
}


void test_and_set(float & min, float & max, Tuple * tuple, int colNo)
{
   float temp_f;

   temp_f = tuple->getFld(colNo, temp_f);
   if (min > temp_f)
      min = temp_f;

   if (max < temp_f)
      max = temp_f;
}



/* ---------------------------------------------------------- */

// OUTPUT THE FOLLOWING FOR EACH ATTRIBUTE
// 1. ATTRNAME
// 2. ATTRTYPE (CONVERT)
// 3. POSITION IN RECORd
// 4. MINVAL
// 5. MAXVAL
// 6. SIZE
// 7. OFFSEt
// 8. NUMBER OF ACCESS METHODS

Status
RelCatalog::rsdumpRelAttributes(std::fstream &outFile, AttrDesc *&attrRecs, int attrCnt,
                                const std::string relName)
{
   int i; 
   Status status = OK;
   IndexDesc *indexRecs = NULL;
   int indexCount;

   for(i = 0; i < attrCnt; i++)
   {
      outFile << '\t' << attrRecs[i].attrName << " ";
      
      switch(attrRecs[i].attrType)
      {
       case attrString  :
	 outFile << attrRecs[i].attrPos << " " ;
	 outFile << "T" << " " ; // attribute type
	 outFile << attrRecs[i].minVal.strVal << " " ;
	 outFile << attrRecs[i].maxVal.strVal << " " ;
	 break;
       case attrReal    :
	 outFile << attrRecs[i].attrPos << " " ;
	 outFile << "R" << " " ;
	 outFile << attrRecs[i].minVal.floatVal << " " ;
	 outFile << attrRecs[i].maxVal.floatVal << " " ;
	 break;
       case attrInteger :
	 outFile << attrRecs[i].attrPos << " " ;
	 outFile << "I" << " ";
	 outFile << attrRecs[i].minVal.intVal << " " ;
	 outFile << attrRecs[i].maxVal.intVal << " " ;
	 break;
       default:
	 std::cerr << "Error don't know how to handle attrNull, attrSymbol" << std::endl;
	 assert(0);
      };

      // NEED TO ADD A COUNT TO INDEXCNT FOR FILESCAN  - AN UGLY KLUGE
      // TO SUPPORT THE CURRENT BOGUS INTERFACE

      outFile << attrRecs[i].attrLen  << " " ;
      outFile << attrRecs[i].attrOffset  << " " ;

      // TO ACCOMODATE THE FILE SCAN
      if (i == attrCnt - 1  )
	 outFile << attrRecs[i].indexCnt + 1 << std::endl;
      else
	 outFile<< attrRecs[i].indexCnt  << std::endl ;

      // NOW GET THE INDEXES SINCE MICHAELL IS EXPECTING THEM
      // TO BE NESTED UNDER THE ATTRIBUTES
      status = MINIBASE_INDCAT->getAttrIndexes( attrRecs[i].relName, attrRecs[i].attrName,
                                                indexCount, indexRecs);
      if (status != OK) break;

      /* ------------------------------------------------------------------ */

      // Generate index information

      // First generate the general information.
      AttrType * in = new AttrType [attrCnt];
      short    * ss = new short    [attrCnt];	// string sizes
      FldSpec  * pl = new FldSpec  [1];		// projection list
      AttrType   in1[1];			// For the duplicate elimination
      short      ss1[1];			// For the duplicate elimination
      int        n_strs = 0;
      Tuple    * tuple = (Tuple *) new char [Tuple::max_size];

      for (int j = 0; j < attrCnt; j++)
      {
	 in[j] = attrRecs[j].attrType;
	 if (in[j] == attrString)
	    ss[n_strs++] = attrRecs[j].attrLen;
      }

      if (attrRecs[i].indexCnt != 0)		// There is atleast one index
      {
	 // Count # of distinct values for the ith attribute.
	 Status s;

	 pl[0].relation = outer;
	 pl[0].offset   = i + 1;	// The offset starts from 1
	 in1[0]		= attrRecs[i].attrType;
	 ss1[0]		= attrRecs[i].attrLen;
	 FileScanIter *fsi = new FileScanIter(relName, in, ss, attrCnt, 1,
					      pl, NULL, s);

	 // Use 8 pages by default to sort <-----------|
	 DuplElim *de = new DuplElim(in1, 1, ss1, fsi, 8, FALSE, s);

	 indexRecs[i].distinctKeys = 0;
	 while ((s = de->get_next(tuple)) != DONE)
	 {
//	    tuple->print(in1);
	    indexRecs[i].distinctKeys++;
	 }

//	 std::cout << "----> DKs = " << indexRecs[i].distinctKeys << std::endl;
	 delete de;		// Automatically deletes fsi.
      }

      delete [] in;
      delete [] ss;
      delete [] pl;
      delete [] tuple;

      /* ------------------------------------------------------------------ */
      
     
      if (indexCount > 0)
	 // BK => now passing attrRecs[i].attrLen <= needed to estimate
	 //       index size (number of leaf pages)
       status = dumpRelIndex(outFile, indexRecs, attrRecs[i].indexCnt,
			     attrRecs[i].attrLen);
      
      if (status != OK) break;

      if (indexRecs)
      {
	 delete [] indexRecs;
	 indexRecs = NULL;
      }
   } 

   // CLEANUP 
   if (indexRecs)
   {
      delete [] indexRecs;
      indexRecs = NULL;
   }

   if (status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

   return OK;
}


/* ---------------------------------------------------------- */

/* ---------------------------------------------------------- */
/*               END OF FILE gen_stats.C                      */
/* ---------------------------------------------------------- */

#if 0
	NOTE THAT THIS CODE IS EXCLUDED BY #if 0
	FROM HERE TO   END  OF FILE -->
/* ---------------------------------------------------------- */

// OUTPUT :
// 1. RELNAME
// 2. ATTRIBUTE COUNT
// 3. NUM TUPLES
// 3. TUPLE SIZE
// 4. NUMBER OF PAGES
// DUMP RELATION CONTENTS TO DISK

Status
RelCatalog::dumpRelation(std::fstream &outFile, RelDesc &relRec, int tupleSize)
{
   Status s   = OK;
   int    cnt = 0;
   RID    rid;
   int    recSize;
   Tuple *tuple = (Tuple *) new char [Tuple::max_size];

   outFile << relRec.relName << " " ;
   outFile << relRec.attrCnt << " " ;

   // determine the number of tuples.
   Heapfile *hf = new HeapFile(relRec.relName, status);
   Scan     *ps = NULL;

   ps = hf->openScan(status);
   while ((status = ps->getNext(rid, (char *) tuple, recSize)) != DONE)
      cnt++;

   delete ps;

   outFile << "[" << cnt << "]";
   outFile << relRec.numTuples << " ";
   outFile << tupleSize << " " ;
   outFile << "[" << hf->n_pages() << "]";
   outFile << relRec.numPages << std::endl ;

   delete hf;
   delete [] (char *) tuple;
   return OK;
}


// DUMP EACH ACCESS METHOD TO DISK
// 1. Index type (name)
// 2. Cost ??
// 3. Record Ordera
// 4. Indicator for index vs scan
// 5. Whether cluterd
// 6. Distinct keys
// 7. index pages
// 8. index selection type (1 = equal only, 2 = equal & range)
// 9. index name

Status
RelCatalog::rsdumpRelIndex(std::fstream &outFile,IndexDesc *&indexRecs,
			     int indexCnt, int attrsize) 
{

   /* All we have to do here is to create a pipe that delivers
    * distinct tuple. And count the number of tuples.
    */
   
   
   int    i     = 0;
   int    cost  = 0;
   int    count = 0;
   int    selectType;
   char   outString[MAXNAME];
   int    indexPlan = 1;
   char * indexName;
   char   accessName[MAXNAME];
   Status status;

   // FOR EACH INDEX
   for(i = 0; i < indexCnt; i++)
   {
      switch(indexRecs[i].accessType)
      {
       case Hash       :
	 strcpy(outString,"Hash");
	 selectType = 1;
	 // Is this correct???  What about index only scans of the whole rel'n??
	 cost = 1;
	 break;
       case B_Index    :
	 strcpy(outString,"B_Index");
	 selectType = 2;
	     // BK & ML => Is this correct??
	     // It is a rough estimate (pretty good though)
	 cost = (sizeof(RID) + attrsize) *
	           indexRecs[i].distinctKeys / MINIBASE_PAGESIZE;
	 break;
      };

      status = MINIBASE_INDCAT->buildIndexName( indexRecs[i].relName,
                                                indexRecs[i].attrName,
                                                indexRecs[i].accessType, indexName);
       
      strcpy(accessName,indexName);
      if (status != OK)
        {
          delete indexName;
          return MINIBASE_CHAIN_ERROR( CATALOG, status );
        }

      // Ok, now we have the name of the index.
      // Scan the entire index, eliminating duplicates.


      outFile << "\t\t" << outString << " ";
      outFile << cost << " ";
       
      switch(indexRecs[i].order)
      {
       case Ascending   :
	 outFile << "A" << " " ;
	 break;

       case Descending  :
	 outFile << "D" << " " ;
	 break;

	case Random      :
	 outFile << "R" << " " ;
	 break;
      };
      outFile << indexPlan <<  " " ;
      outFile << indexRecs[i].clustered << " " ;
      outFile << indexRecs[i].distinctKeys << " ";

      // BK we are using a height of "1" for now.  Should change in future
      outFile << 1 << " ";
      outFile << selectType << " ";
      outFile << accessName << std::endl;
      delete indexName;
   }
  return OK;
}


#endif
