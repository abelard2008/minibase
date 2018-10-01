#include "catalog.h"
#include "maincatalog.h"
extern "C" int strcasecmp( const char*, const char* );

//-------------------------------------------------
//CONSTRUCTOR
//-------------------------------------------------

RelCatalog::RelCatalog(Status &status):
	 HeapFile(RELCATNAME, status) 
{ 
  tuple = (Tuple *) new char [Tuple::max_size];
  attrs = new AttrType[5];

  attrs[0] = attrString;
  attrs[1] = attrInteger;
  attrs[2] = attrInteger;
  attrs[3] = attrInteger;
  attrs[4] = attrInteger;

  str_sizes = new short;
  str_sizes[0] = MAXNAME;

  tuple->setHdr(5, attrs, str_sizes);

}


//------------------------------------------------------------
// GET INFO ON A SINGLE RELATION
// NOTE THAT RELFOUND MUST!! BE RETURNED IF RECORD NOT FOUND
//------------------------------------------------------------

Status RelCatalog::getInfo(const std::string relation, RelDesc &record)
{
  Status status;
//  char recPtr[sizeof(RelDesc)];   // BK
  int recSize;
  RID rid;
  Scan *pscan = NULL; 

  // CHECK PARM

  if (relation.empty())
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::MISSING_PARAM );

  //  OPEN CATALOG SCAN

  pscan = openScan(status);
  if (!pscan)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );

  if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // LOCATE RELATION RECORD BY SCANNING
  // NOTE THAT RELNOTFOUND MUST!! BE RETURNED IF RECORD NOT FOUND

  while (1)
   {
//     status = pscan->getNext(rid, recPtr, recSize);
     status = pscan->getNext(rid, (char*)tuple, recSize);
     if (status == DONE )
      {
	delete pscan;
	return RELNOTFOUND;
      }

     else if (status == OK)
      {
	if(tuple->size() != recSize)
	  {
            delete pscan;
            return MINIBASE_FIRST_ERROR( CATALOG, Catalog::BAD_RECORD_SIZE );
	  }

	read_tuple(tuple, record);
	if(!strcasecmp(record.relName, relation.c_str()))
	 {
	  delete pscan;
	  return OK;
         }
      }

     else // AN ERROR FROM GETNEXT
      {
	delete pscan;
        return MINIBASE_CHAIN_ERROR( CATALOG, status );
      }


  } // END WHILE

  // CLEAN UP

 delete pscan;
}


//-----------------------------------------------
// ADD RECORD TO RELATION CATALOG
//-----------------------------------------------

Status RelCatalog::addInfo(RelDesc record)
{
  RID rid;
  Status  status;

//  int len = strlen(record.relName);
//  memset(&record.relName[len], 0, sizeof record.relName - len);

  make_tuple(tuple, record);
//  status = insertRecord((char *)&record, sizeof(RelDesc), rid);
  status = insertRecord((char*)tuple, tuple->size(), rid);

  return status;
}

//-------------------------------------------------------------
// REMOVE A RECORD FROM RELATION CATALOG
//------------------------------------------------------------

Status RelCatalog::removeInfo(const std::string relation)
{
  Status status;
  RID rid;
  Scan *pscan = NULL; 
//  char recPtr[sizeof(RelDesc)];     // BK
  int recSize;
  RelDesc record;

  // CHECK PARAMETER

  if (relation.empty())
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::MISSING_PARAM );

  // OPEN SCAN

  pscan = openScan(status);
  if (!pscan)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
  if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // LOCATE AND DELETE CATALOG INFO 

  while (1)
   {
//     status = pscan->getNext(rid, recPtr, recSize);
     status = pscan->getNext(rid, (char*)tuple, recSize);
     if (status == DONE) 
      {
	delete pscan;
        return MINIBASE_FIRST_ERROR( CATALOG, Catalog::REL_NOT_FOUND );
      }
     else if (status == OK)
      {
// BK
//        assert(sizeof(RelDesc) == recSize);
//        memcpy(&record, recPtr, recSize);
	assert(tuple->size() == recSize);
	read_tuple(tuple, record);

	if(!strcasecmp(record.relName, relation.c_str()))
          {
	   delete pscan;
	   status = deleteRecord(rid);	
	   if(status != OK)
             {
               delete pscan;
               return MINIBASE_CHAIN_ERROR( CATALOG, status );
             }
	   return OK;
          }
      }
     else // ERROR IN GET NEXT
       {
         delete pscan;
         return MINIBASE_CHAIN_ERROR( CATALOG, status );
       }

  } // end while

 delete pscan;
 return OK;
}


//--------------------------------------------------
// MAKE_TUPLE
//--------------------------------------------------
// Tuple must have been initialized properly in the 
// constructor

Status RelCatalog::make_tuple(Tuple* tuple, RelDesc record)
{
  tuple->setFld(1, record.relName);
  tuple->setFld(2, record.attrCnt);
  tuple->setFld(3, record.indexCnt);
  tuple->setFld(4, record.numTuples);
  tuple->setFld(5, record.numPages);
  return OK;
}




//--------------------------------------------------
// READ_TUPLE
//--------------------------------------------------

Status RelCatalog::read_tuple(Tuple* tuple, RelDesc& record)
{
  char* tempptr;
  tuple->getFld(1, tempptr);
  strcpy(record.relName, tempptr);
  tuple->getFld(2, record.attrCnt);
  tuple->getFld(3, record.indexCnt);
  tuple->getFld(4, record.numTuples);
  tuple->getFld(5, record.numPages);
  return OK; 
}




//----------------------------------------------
// DESTRUCTOR
//----------------------------------------------

RelCatalog::~RelCatalog()
{
  delete [] attrs;
  delete str_sizes;
  delete [] tuple;
}


