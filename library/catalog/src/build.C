//--------------------------------------
// P. Feakins May 1995
//--------------------------------------
#include "maincatalog.h"
#include "catalog.h"


//-----------------------------------------------------------------
// CREATES AN INDEX FOR A RELATIONa
// - take            
//   1. relation name
//   2. attribute name
//   3. index type
// - does
//   1. calls indexCatalog::addIndex to
//      - make index catalog entry
//      - load the index from data file
//   2. updates the RelCatalog
//----------------------------------------------------------------

Status RelCatalog::addIndex(const std::string relation, const std::string attrName, IndexType accessType, int /*buckets*/ )
{
  Status status;
  RelDesc rd;

  // CHECK PARMS

  if (relation.empty() || attrName.empty())
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::MISSING_PARAM );

  // GET RELATION DATA

  if ((status = getInfo(relation, rd)) != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // CREATE INDEX FILE

  if ((status = MINIBASE_INDCAT->addIndex(relation, attrName,accessType)) != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // MODIFY INDEXCNT IN RELCAt

  rd.indexCnt++;

  if ((status = removeInfo(relation)) != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  if ((status = addInfo(rd)) != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  return OK;
}



//--------------------------------------------------------------------------------------
// ADDS AN INDEX TO A RELATIO
// -takes
//  1. realtion 
//  2. single attribute
//  3. indextype
// - does
//  1. updates attribute catalog (indexcount)
//  2. adds entry to index catalog
//  3. scans underlying datafile and builds index
// - note
//   - only supports index on a single attribute currently
//-------------------------------------------------------------------------------------

Status IndexCatalog::addIndex(const std::string relation, const std::string attrName, IndexType accessType, int buckets)
{
  Status status;
  RID rid;
  IndexDesc indexRec;
  AttrDesc attrRec;
  int intKey;
  float floatKey;
  char *charKey;
  int attrCnt;
  void *key = 0;      
  int recSize;
 // THESE POINTERS NEED TO BE DELETED ON RETURN
  HeapFile* datafile = NULL;
  char* indexName = NULL;
  Tuple* tuple = NULL;
  BTreeFile *btree = NULL;
  /*  LinearHashingFile *hash = NULL;  */
  Scan *pscan = NULL;
  AttrType *typeArray = NULL;
  short *sizeArray = NULL;

  // CHECK PARM 

  if (relation.empty() || attrName.empty())
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::MISSING_PARAM );

  // CHECK FOR EXISTING INDEX

  status = getInfo(relation, attrName, accessType, indexRec);

  if(status == OK)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::INDEX_EXISTS );
  else if (status != INDEXNOTFOUND)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // GET ATTRIBUTE INFO 

  status = MINIBASE_ATTRCAT->getInfo(relation, attrName, attrRec);
  if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // UPDATE ATTRIBUTE INFO

  attrRec.indexCnt++;

  status = MINIBASE_ATTRCAT->removeInfo(relation, attrName);
  if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  status = MINIBASE_ATTRCAT->addInfo(attrRec);
  if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // BUILD INDEX FILE NAME

  status = buildIndexName(relation, attrName, accessType, indexName);
  if(status != OK)
    {
      delete indexName;
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

  // ADDED BY BILL KIMMEL - DELETE LATER
  std::cout << "Index name is " << indexName << std::endl;

  // IF BTREE

  if(accessType == B_Index)
   {
     btree = new BTreeFile(status, indexName, attrRec.attrType, attrRec.attrLen);

     if(!btree)
       {
         delete indexName;
         return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
       }
     if(status != OK)
       {
         delete btree;
         delete indexName;
         return MINIBASE_CHAIN_ERROR( CATALOG, status );
       }
   } 

  // IF HASH

/*
  else if(accessType == Hash) 
   {
     hash = new LinearHashingFile(status, indexName, attrRec.attrLen, attrRec.attrType,
				     buckets);

     if(!hash)
       {
         delete indexName;
         return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
       }
     if(status != OK)
       {
         delete hash;
         delete indexName;
         return MINIBASE_CHAIN_ERROR( CATALOG, status );
       }
   }
*/

  // ADD ENTRY IN INDEXCAT

  strcpy(indexRec.relName,relation.c_str());
  strcpy(indexRec.attrName,attrName.c_str());
  indexRec.accessType = accessType;
  if (accessType == B_Index)
    indexRec.order = Ascending;
  else
    indexRec.order = Random;

  indexRec.distinctKeys = DISTINCTKEYS;
  indexRec.clustered = 0;  // 0 means non-clustered!!!!

  indexRec.indexPages  = INDEXPAGES;

  if ((status = addInfo(indexRec)) != OK)
    {
      if(btree)
          delete btree;
      /*  else if (hash)    */
          /*  delete hash;  */
      delete indexName;
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

  // PREPARE TO SCAN DATA FILE

  datafile =  new HeapFile(relation, status); 
  if (!datafile)
    {
      if(btree)
          delete btree;
      /*  else if(hash)  */
          /*  delete hash;  */
      delete indexName;
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
    }

  pscan = datafile->openScan(status);
  if(!pscan)
    { 
      if(btree)
          delete btree;
      /*  else if (hash)  */
          /*  delete hash;  */
      delete indexName;
      delete datafile;
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
    }

  // PREPARE TUPLE

  status = MINIBASE_ATTRCAT->getTupleStructure(relation, attrCnt, typeArray, sizeArray);
  if(status != OK)
    {
      if(btree)
          delete btree;
      /*  else if (hash)  */
          /*  delete hash;  */
      delete pscan;
      delete indexName;
      delete datafile;
      delete typeArray;
      delete sizeArray;
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

  tuple = (Tuple *) new char[Tuple::max_size];
  if(!tuple)
    {
      if(btree)
          delete btree;
      /*  else if (hash)  */
          /*  delete hash;  */
      delete pscan;
      delete indexName;
      delete datafile;
      delete typeArray;
      delete sizeArray;
      delete tuple;
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
    }
  
  status = tuple->setHdr((short) attrCnt, typeArray, sizeArray);
  recSize = tuple->size();

  // NOW PROCESS THE HEAPFILE AND INSERT KEY,RID INTO INDEX

  while(1)
   {
     status = pscan->getNext(rid, (char *) tuple, recSize);
     if(status == DONE )
      { 
        status = OK;
        break;
      }
     else if (status != OK)
      { 
        break;  
      }

     // PULL OUT THE KEY VALUE FROM HEAPFILE RECORD

     if(attrRec.attrType == attrInteger)
      {
       intKey = tuple->getFld(attrRec.attrPos, intKey);
       key = (char *) &intKey;
      }
     else if(attrRec.attrType == attrReal)
      {
       floatKey = tuple->getFld(attrRec.attrPos, floatKey);
       key = (char *) &floatKey;
      }
     else if(attrRec.attrType == attrString)
      {
       charKey = tuple->getFld(attrRec.attrPos, charKey);
       key = charKey;
      }

    // NOW INSERT RECORD INTO INDEX

     if(accessType == B_Index)
       status = btree->insert(key, rid); 
     /*  else if(accessType == Hash)  */
       /*  status = hash->insert(key, rid);  */

     if(status != OK)
      { 
       break;
      }
   }

  // AT END OF FILE - CLEAN UP

  if(btree)
     delete btree;
  /*  else if (hash)  */
   /*  delete hash;  */
  delete pscan;
  delete datafile;
  delete indexName;
  delete typeArray;
  delete sizeArray;
  delete tuple;

  if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  return OK;
}
