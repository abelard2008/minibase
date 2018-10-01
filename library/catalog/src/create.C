//----------------------------------------
// P. Feakins May 1995
//---------------------------------------

#include "maincatalog.h"
#include "catalog.h"
extern "C" int strcasecmp( const char*, const char* );

//------------------------------------------------------------
// CREATES A NEW RELATION
// - takes
//   1. relation name
//   2. count of attributes
//   3. an array of attribute names, types,lengths
// - creates
//   1. relcatalog entry
//   2. attrcatalog entries
//   3. datafile (heapfile with the relation name)
//-------------------------------------------------------------

Status RelCatalog::createRel(const std::string relation, int attrCnt,
			     attrInfo attrList[])
{
  Status status;
  HeapFile *rel;
  RelDesc rd;
  AttrDesc ad;
  int tupleWidth = 0;
  int offset = 0;
  int i; 

  // CHECK PARMS

  if (relation.empty() || attrCnt < 1)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::MISSING_PARAM );

  if (strlen(relation.c_str()) >= sizeof rd.relName)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::REL_NAME_TOO_LONG );
  
  // MAKE SURE THE RELATION DOESN'T ALREADY EXIST

  status = getInfo(relation, rd);
  if (status == OK)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::REL_EXISTS );

  if (status != RELNOTFOUND)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );


  // MAKE SURE THERE ARE NO DUPLICATE ATTRIBUTE NAMES


    for(i = 0; i < attrCnt; i++)
     {
      if(attrList[i].attrType == attrString)
         tupleWidth += sizeof(std::string);
      else if(attrList[i].attrType == attrInteger)
         tupleWidth += sizeof(int);
      else if(attrList[i].attrType == attrReal)
         tupleWidth += sizeof(float);

      for(int j = 0; j < i; j++)
          if (strcasecmp(attrList[i].attrName, attrList[j].attrName) == 0)
              return MINIBASE_FIRST_ERROR( CATALOG, Catalog::DUP_ATTRS );
      }
 
  // CHECK LEGIT TUPLE SIZE

  if (tupleWidth > MINIBASE_PAGESIZE)            // should be more strict
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::TUPLE_TOO_BIG );

  // std::cerr << "Creating relation " << relation << endl;

  // INSERT INFORMATION ABOUT RELATION

  strcpy(rd.relName, relation.c_str());
  rd.attrCnt = attrCnt;
  rd.indexCnt = 0;
  rd.numTuples = NUMTUPLESFILE;
  rd.numPages = NUMPAGESFILE;

  if ((status = addInfo(rd)) != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // INSERT INFORMATION ABOUT ATTRIBUTES

  strcpy(ad.relName, relation.c_str());

  for(i = 0; i < attrCnt; i++)
   {
    if (strlen(attrList[i].attrName) >= sizeof ad.attrName)
        return MINIBASE_FIRST_ERROR( CATALOG, Catalog::ATTR_NAME_TOO_LONG );

    strcpy(ad.attrName, attrList[i].attrName);
    ad.attrOffset = offset;
    ad.attrType = attrList[i].attrType;
    ad.indexCnt = 0;
    ad.attrPos = i + 1;   // field position in the record

    if(attrList[i].attrType == attrString)
	{
         ad.attrLen = attrList[i].attrLen;
  	 strcpy((char *)ad.maxVal.strVal, "Z");
  	 strcpy((char *)ad.minVal.strVal, "A");
//	 ad.maxVal.strVal = MAXSTRINGVAL;
//	 ad.minVal.strVal, MINSTRINGVAL;
        }
    else if(attrList[i].attrType == attrInteger)
       {
         ad.attrLen = sizeof(int);
	 ad.minVal.intVal = MINNUMVAL;
	 ad.maxVal.intVal = MAXNUMVAL;
       }
    else if(attrList[i].attrType == attrReal)
       {
         ad.attrLen = sizeof(float);
         ad.minVal.floatVal = MINNUMVAL;
	 ad.maxVal.floatVal = MAXNUMVAL;
       }

    if ((status = MINIBASE_ATTRCAT->addInfo(ad)) != OK)
        return MINIBASE_CHAIN_ERROR( CATALOG, status );

    offset += ad.attrLen;
  }

  // NOW CREATE HEAPFILE

  rel = new HeapFile(relation, status);

  if(!rel)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
  if (status != OK)
    {  
      delete rel;
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

  delete rel;

  return OK;
}
