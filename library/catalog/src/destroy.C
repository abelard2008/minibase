//-------------------------------------------------------------
// P. Feakins May 1995
//-------------------------------------------------------------

#include "maincatalog.h"
#include "catalog.h"
extern "C" int strcasecmp( const char*, const char* );

//--------------------------------------------------------------
// DESTROYS A RELATION INCUDING DATA/INDEXES/CATALOG ENTRIES
// -takes
//  1. relation name
// -does
//  1. calls IndexCatalog::dropRelation
//     - removes inmdex catalog info
//     - deletes index files
//  2. calls AttrCatalog::dropRelation
//     - remove attribute catalog entries
//  3. removes RelCatalog entry
//--------------------------------------------------------------

Status RelCatalog::destroyRel(const std::string relation)
{
  Status status;
  HeapFile *heap;

  // CHECK PARMS

  if (relation.empty() || !strcasecmp(relation.c_str(), RELCATNAME)
      || !strcasecmp(relation.c_str(), ATTRCATNAME))
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::MISSING_PARAM );

  // DELETE  INDEXES

  if ((status = MINIBASE_INDCAT->dropRelation(relation)) != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  //  DELETE ATTRIBUTE ENTRIES

  if ((status = MINIBASE_ATTRCAT->dropRelation(relation)) != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // DELETE ENTRY FROM RELCAT

  if ((status = removeInfo(relation)) != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // DESTROY DATAFILE FILE

  if(!(heap = new HeapFile(relation,status)))
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
  if(status != OK)
    {
      delete heap;
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

  status = heap->deleteFile();
  if(status != OK)
    {
      delete heap;
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

  // CLEANUP

  delete heap;

  return OK;

}

//-----------------------------------------------
// DROP ALL ATTRIBUTE FOR RELATION
// - does
// 1. removes catalog entry for all attributes in relation
//------------------------------------------------

Status AttrCatalog::dropRelation(const std::string relation)
{
  Status status;
  AttrDesc *attrs;
  int attrCnt;

  
  if (relation.empty())
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::MISSING_PARAM );

  // GET ATTRIBUTE INFORMATION

  if ((status = getRelInfo(relation, attrCnt, attrs)) != OK)
    {
      delete [] attrs;
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

  // REMOVE ENTRIES FROM CATALOG

  for(int i = 0; i < attrCnt; i++) 
      if ((status = removeInfo(relation, attrs[i].attrName)) != OK)
        {
          delete [] attrs;
          return MINIBASE_CHAIN_ERROR( CATALOG, status );
        }


  // CLEAN UP

  delete [] attrs;
  return OK;
}


//--------------------------------------------------
// DROP ALL INDEXES FOR A RELATION
// -does
//  1. calls IndexCAtalog::dropIndex for each index to:
//     - remove catalog entries
//     - delete underlying index file
//------------------------------------------------------

Status IndexCatalog::dropRelation(const std::string relation)
{
  Status status;
  IndexDesc *ind = 0;
  int indexCnt;

  if (relation.empty())
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::MISSING_PARAM );

  // GET INDEXES INFORMATION

  if ((status = getRelInfo(relation, indexCnt, ind)) != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );


  // DELETE EACH INDEX BY CALLING DROP INDEX FOR EACH
  // NOTE THAT RELCATALOG DROPINDEX IS NOT CALLED AS THE
  // RELCATATLOG ENTRY WILL SIMPLY BE DELETED ANYWAY

  for(int i = 0; i < indexCnt; i++)
      if ((status = dropIndex(relation, ind[i].attrName, ind[i].accessType)) != OK)
        {
          delete [] ind;
          return MINIBASE_CHAIN_ERROR( CATALOG, status );
        }

   // CLEANUP 

   delete [] ind;
   return OK;
}
