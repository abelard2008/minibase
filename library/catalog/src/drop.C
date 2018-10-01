//------------------------------------
// P. Feakins MMay 1995
//------------------------------------
#include "maincatalog.h"
#include "catalog.h"

//-------------------------------------------------------------
// DROPS AN INDEX      
// - takes
//   1. realtion name
//   2. attribute name (one!)
//   3. accesstype
// - does
//   1. calls IndexCatalog::dropIndex to:
//      1. modify attribute catalog
//      2. remove index catalog entry
//      3. delete underlying indexfile
//   2. modifies relation index count
//
//-------------------------------------------------------------

Status RelCatalog::dropIndex(const std::string relation, const std::string attrName, IndexType accessType)
{
  Status status;
  RelDesc rd;

  if (relation.empty() || attrName.empty())
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::MISSING_PARAM );

  // GET RELATION DATA

  if ((status = getInfo(relation, rd)) != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // DESTROY INDEX FILE

  if ((status = MINIBASE_INDCAT->dropIndex(relation, attrName, accessType)) != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // MODIFY INDEXCNT IN RELCAT

  rd.indexCnt--;

  if ((status = removeInfo(relation)) != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  if ((status = addInfo(rd)) != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // CLEANUP

  return OK;
}

//---------------------------------------------------------------------
// DROP THE PHYSICAL INDEX AND UPDATE INDEX/ATTRIBUTE CATALOG ENTRIES
// - takes
//   1. relation name
//   2. attribute name
//   3. accesstype
// - does
//   1. updates attribute record index count
//      - done here now since there may be multiple attributes in key later
//        and we would exxpect indexcatalog to keep track of them
//   2. removes indexcatalog entry
//   3. deletes underlying index file
//---------------------------------------------------------------------

Status IndexCatalog::dropIndex(const std::string relation, const std::string attrName, IndexType accessType)
{
  Status status;
  IndexDesc indexRec;
  AttrDesc attrRec;
  // FOLLOWING PTRS SHOULD BE DELETED BEFORE RETURN
  BTreeFile* btree = NULL;
  /*  LinearHashingFile *hash = NULL;  */
  char *indexName = NULL;

  if (relation.empty() || attrName.empty())
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::MISSING_PARAM );

  // GET ATTRIBUTE INFORMATION

  if ((status = getInfo(relation, attrName, accessType, indexRec)) != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  if((status = MINIBASE_ATTRCAT->getInfo(relation, attrName, attrRec)) != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

 // UPDATE ATTRIBUTE CATALOG 

  attrRec.indexCnt--;

  if((status = MINIBASE_ATTRCAT->removeInfo(relation,attrName)) != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  if((status = MINIBASE_ATTRCAT->addInfo(attrRec)) != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // BUILD INDEX NAME

  status = buildIndexName(relation, attrName, accessType, indexName);
  if(status != OK)
    {
      delete indexName;
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

  // DESTROY FILE

  //  BTREE DESTROY CODE

  if(indexRec.accessType == B_Index)
    {  
       btree = new BTreeFile(status, indexName);

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
 
       // DESTROY BTREE FILE

       status = btree->destroyFile();
       if(status != OK)
         {
           delete btree;
           delete indexName;
           return MINIBASE_CHAIN_ERROR( CATALOG, status );
         }
    
       delete btree;
     }


  // HASH  DESTROY CODE

/*
  if(indexRec.accessType == Hash)
    {
       // GET HASH FILE
       
       hash = new LinearHashingFile(status, indexName);
    
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
      
       // REMOVE HASH FILE

       status = hash->destroyFile();
       if(status != OK)
         {
           delete hash;
           delete indexName;
           return MINIBASE_CHAIN_ERROR( CATALOG, status );
         }
    
       delete hash;
   }
*/

  // MODIFY ENTRY IN ATTRCAT

  status = removeInfo(relation, attrName, accessType);
  if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // CLEANUP

  delete indexName;
  return OK;
}
