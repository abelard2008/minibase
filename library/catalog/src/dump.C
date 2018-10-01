#include "maincatalog.h"
#include "catalog.h"

// dumps a catalog to disk in a specific format as a temporary interafce to  the optimizer
// 1. data for a relation
// 2. nested inside each relation, each attribute
// 3. nested inside each attribute any relevant accessmethods

Status RelCatalog::dumpCatalog(const std::string filename)      
{
  RelDesc relRec;
  AttrDesc *attrRecs = NULL;
//  IndexDesc *indexRecs;
  std::fstream outFile;
  Scan *pscan = NULL;
  int relCount = 0 ;
  int recSize;
//  char recPtr[MINIBASE_PAGESIZE];
  RID rid;
//  int tupleSize;
//  int numAccess;
  int indexPlan = 0;
  Status status;
 
  pscan = openScan(status);
  if(! pscan)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );

// OPEN BOGUSCCATALOGS

  outFile.open(filename, std::ios::out);
  // CHECK FOR BAD OPEN HERE!!!!!!!!!!!!!11
  outFile << "        " << std::endl ;       // Leave enough room for a big number of relations...


// SCAN THE RELATION CATALOG (A HEAPFILE)

  while (1)
   {
   // get each relation

//    status = pscan->getNext(rid, recPtr, recSize);
    status = pscan->getNext(rid, (char*)tuple, recSize);
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
//       assert(sizeof(RelDesc) == recSize);
//       memcpy(&relRec, recPtr, recSize);
       assert(tuple->size() == recSize);
       read_tuple(tuple, relRec);

       // get all attributes for the relation
       status = MINIBASE_ATTRCAT->getRelInfo(relRec.relName, relRec.attrCnt, attrRecs);
       if(status != OK)
	 break;

       int tupleSize = 0;
       for(int i = 0 ; i < relRec.attrCnt; i++)
	 tupleSize = tupleSize + attrRecs[i].attrLen;

       //  dump realtion record to disk
       status = dumpRelation(outFile, relRec, tupleSize);
       if(status != OK)
	  break;

       // DUMP ATTRIBUTES TO DISK -> ACCESSMETHODS WILL ALSO BE DUMPED

       status = dumpRelAttributes(outFile, attrRecs, relRec.attrCnt);
       if(status != OK)
	  break;

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

// OUPUT :
// 1. RELNAME
// 2. ATTRIBUTE COUNT
// 3. NUM TUPLES
// 3. TUPLE SIZE
// 4. NUMBER OF PAGES
// DUMP RELATION CONTENTS TO DISK
Status RelCatalog::dumpRelation(std::fstream &outFile, RelDesc &relRec, int tupleSize)
 {
  outFile << relRec.relName << " " ;
  outFile << relRec.attrCnt << " " ;
  outFile << relRec.numTuples << " ";
  outFile << tupleSize << " " ;
  outFile << relRec.numPages << std::endl ;
  return OK;
 }

// OUTPUT THE FOLLOWING FOR EACH ATTRIBUTE
// 1. ATTRNAME
// 2. ATTRTYPE (CONVERT)
// 3. POSITION IN RECORd
// 4. MINVAL
// 5. MAXVAL
// 6. SIZE
// 7. OFFSEt
// 8. NUMBER OF ACCESS METHODS
Status RelCatalog::dumpRelAttributes(std::fstream &outFile,AttrDesc *&attrRecs, int attrCnt)  
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
      case attrString  : outFile << attrRecs[i].attrPos << " " ;
                         outFile << "T" << " " ; // attribute type
		   	 outFile << attrRecs[i].minVal.strVal << " " ;
		         outFile << attrRecs[i].maxVal.strVal << " " ;
		         break;
      case attrReal    : outFile << attrRecs[i].attrPos << " " ;
			 outFile << "R" << " " ;
			 outFile << attrRecs[i].minVal.floatVal << " " ;
		         outFile << attrRecs[i].maxVal.floatVal << " " ;
		         break;
      case attrInteger : outFile << attrRecs[i].attrPos << " " ;
			 outFile << "I" << " ";
			 outFile << attrRecs[i].minVal.intVal << " " ;
		         outFile << attrRecs[i].maxVal.intVal << " " ;
		         break;
      default: break;
     }
   // NEED TO ADD A COUNT TO INDEXCNT FOR FILESCAN  - AN UGLY KLUGE
   // TO SUPPORT THE CURRENT BOGUS INTERFACE

    outFile << attrRecs[i].attrLen  << " " ;
    outFile << attrRecs[i].attrOffset  << " " ;

    // TO ACCOMODATE THE FILE SCAN
    if( i == attrCnt - 1  )
      outFile << attrRecs[i].indexCnt + 1 << std::endl;
    else
      outFile<< attrRecs[i].indexCnt  << std::endl ;

    // NOW GET THE INDEXES SINCE MICHAELL IS EXPECTING THEM TO BE NESTED UNDER THE ATTRIBUTES
    status = MINIBASE_INDCAT->getAttrIndexes( attrRecs[i].relName, attrRecs[i].attrName,
                                              indexCount, indexRecs);
    if(status != OK)
      break;
    if(indexCount > 0)
       // BK => now passing attrRecs[i].attrLen <= needed to estimate index size (number of leaf pages)
       status = dumpRelIndex(outFile, indexRecs, attrRecs[i].indexCnt, attrRecs[i].attrLen);
      
    if(status != OK)
      break;

    if(indexRecs)
      {
	delete [] indexRecs;
	indexRecs = NULL;
      }

  } 

   // CLEANUP 
 
    if(indexRecs)
	{
	  delete [] indexRecs;
	  indexRecs = NULL;
        }
    if(status != OK)
        return MINIBASE_CHAIN_ERROR( CATALOG, status );

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
Status RelCatalog::dumpRelIndex(std::fstream &outFile,IndexDesc *&indexRecs, int indexCnt, int attrsize) 
 {
  int i = 0;
  int cost = 0;
  int selectType = 0;
  char outString[MAXNAME];
  int indexPlan = 1;
  char *indexName;
  char accessName[MAXNAME];
  Status status;

  // FOR EACH INDEX

  for(i = 0; i < indexCnt; i++)
   {
       switch(indexRecs[i].accessType)
       {
	 case Hash       : strcpy(outString,"Hash");
		           selectType = 1;
			   // Is this correct???  What about index only scans of the whole rel'n??
			   cost = 1;
		           break;
         case B_Index    : strcpy(outString,"B_Index");
		           selectType = 2;
			   // BK & ML => Is this correct??  It is a rough estimate (pretty good though)
                           cost = (sizeof(RID) + attrsize) * indexRecs[i].distinctKeys / MINIBASE_PAGESIZE;
		           break;
	 default: break;
       };

       status = MINIBASE_INDCAT->buildIndexName( indexRecs[i].relName,indexRecs[i].attrName,
                                                 indexRecs[i].accessType, indexName);
       
       strcpy(accessName,indexName);
       if(status != OK)
         {
           delete indexName;
           return MINIBASE_CHAIN_ERROR( CATALOG, status );
         }

       outFile << "\t\t" << outString << " ";
       outFile << cost << " ";
       
       switch(indexRecs[i].order)
       {
	case Ascending   : outFile << "A" << " " ;
			   break;

	case Descending  : outFile << "D" << " " ;
			   break;

	case Random      : outFile << "R" << " " ;
			   break;
       };
       outFile << indexPlan <<  " " ;
       outFile << indexRecs[i].clustered << " " ;
       outFile << indexRecs[i].distinctKeys << " ";
//       outFile << indexRecs[i].indexPages  << " " ;
       // BK we are using a height of "1" for now.  Should change in future
       outFile << 1 << " ";
       outFile << selectType << " ";
       outFile << accessName << std::endl;
       delete indexName;
   }
   return OK;
 }
