#include "maincatalog.h"
#include "catalog.h"
extern "C" int strcasecmp( const char*, const char* );

//---------------------------------------------------------
// CONSTRUCTOR
//---------------------------------------------------------

IndexCatalog::IndexCatalog(Status &status) :
	 HeapFile(INDEXCATNAME,status)
{
  tuple = (Tuple *) new char [Tuple::max_size];
  attrs = new AttrType[7];
  
  attrs[0] = attrString;
  attrs[1] = attrString;
  attrs[2] = attrInteger;
  attrs[3] = attrInteger; // 0 = None
                          // 1 = B_Index
                          // 2 = Linear Hash

  attrs[4] = attrInteger; // 0 = Ascending
                          // 1 = Descending
                          // 2 = Random
  attrs[5] = attrInteger;
  attrs[6] = attrInteger;


  str_sizes = new short[2];
  str_sizes[0] = MAXNAME;
  str_sizes[1] = MAXNAME;

  tuple->setHdr(7, attrs, str_sizes);
}


//--------------------------------------------------------------------
// GET ALL INDEXES FOR A RELATION
//---------------------------------------------------------------------

Status IndexCatalog::getRelInfo(const std::string relation, int &indexCnt, IndexDesc *&indexes)
{
  RelDesc record;
  Status status;
//  char recPtr[sizeof(IndexDesc)];  // BK
  int recSize;
  RID rid;
  Scan *pscan = NULL;
  int count = 0;

  // CHECK THE RELATION

  if (relation.empty())
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::MISSING_PARAM );

  status = MINIBASE_RELCAT->getInfo(relation, record);
  if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // SET INDEX COUNT BY REFERENCE 

  indexCnt = record.indexCnt;

  if (indexCnt == 0)
    {
      indexes = 0;
      return OK;
    }

  // OPEN SCAN

  pscan = openScan(status);
  if(! pscan)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
  if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // ALLOCATE INDEX ARRAY

  if(!(indexes = new IndexDesc[indexCnt]))
    {
      delete pscan;
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
    }

  // SCAN THE FILE

  while(1) 
   {
//    status = pscan->getNext(rid, recPtr, recSize);
     status = pscan->getNext(rid, (char*)tuple, recSize);
    if(status != OK)
       break;

//    assert(sizeof(IndexDesc) == recSize);
//    memcpy(&indexes[count], recPtr, recSize);
     assert(tuple->size() == recSize);
     read_tuple(tuple, indexes[count]);

     if(!strcasecmp (indexes[count].relName, relation.c_str()))
      count++;

    if(count == indexCnt)  // IF ALL INDEXES FOUND
      break;
   }

  // CLEANUP

  delete pscan; 

  if (status == DONE )  // NOT ALL INDEXES WERE FOUND
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::INDEX_NOT_FOUND );
  else if (status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  return OK;
}

//--------------------------------------------------------
// GET ALL INDEXES FOR A PARTICULAR ATTRIBUTE
//--------------------------------------------------------

Status IndexCatalog::getAttrIndexes(const std::string relation, const std::string attrName,
			 int &indexCnt, IndexDesc *&indexes)
{
  AttrDesc record;
  Status status;
//  char recPtr[sizeof(IndexDesc)];
  int recSize;
  RID rid;
  Scan *pscan = NULL;
  int count = 0;

  // CHECK RELATION

  if (relation.empty())
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::MISSING_PARAM );

  status = MINIBASE_ATTRCAT->getInfo(relation, attrName, record);
  if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // ASSIGN INDEX COUNT

  indexCnt = record.indexCnt;
  if(indexCnt == 0)
     return status;

  // OPEN SCAN

  pscan = openScan(status);
  if(! pscan)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );

  if(status != OK)
    {
      delete pscan;
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

  // ALLOCATE ARRAY

  if(!(indexes = new IndexDesc[indexCnt]))
    {
      delete pscan;
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

  // SCAN FILE

  while(1) 
   {
//    status = pscan->getNext(rid, recPtr, recSize);

     status = pscan->getNext(rid, (char*)tuple, recSize);
    if(status != OK)
	break;

//    assert(sizeof(IndexDesc) == recSize);
//    memcpy(&indexes[count], recPtr, recSize);

     assert(tuple->size() == recSize);
     read_tuple(tuple, indexes[count]);

     if(!strcasecmp (indexes[count].relName, relation.c_str()) 
        && !strcasecmp(indexes[count].attrName,attrName.c_str()))
      count++;

    if(count == indexCnt)  // if all indexes found
        break; 
   }

  // CLEANUP

  delete pscan;  
  if (status == DONE ) // NOT ALL INDEXES FOUND
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::INDEX_NOT_FOUND );
  else if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  return OK;
}

//------------------------------------------------------
// GET INFO ON A SINGLE INDEX
// NOTE: MUST RETURN INDEXNOTFOUND IF NOT FOUND !!!
//------------------------------------------------------

Status IndexCatalog::getInfo(const std::string relation, const std::string attrName, IndexType accessType,
			      IndexDesc &record)
{
  Status status;
//  char recPtr[sizeof(IndexDesc)];
  int recSize;
  RID rid;
  Scan *pscan = NULL; 

  // CHECK PARAMETER

  if (relation.empty() || attrName.empty())
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::MISSING_PARAM );

 // OPEN SCAN

  pscan = openScan(status);
  if (!pscan)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
  if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

 // SCAN FILE

  while (1)
   {
//     status = pscan->getNext(rid, recPtr, recSize);

     status = pscan->getNext(rid, (char*)tuple, recSize);

     if (status == DONE )
        break; 
     else if (status == OK)
      {
//        assert(sizeof(IndexDesc) == recSize);
//        memcpy(&record, recPtr, recSize);

	assert(tuple->size() == recSize);
	read_tuple(tuple, record);

	if(!strcasecmp (record.relName, relation.c_str()) 
	   && !strcasecmp(record.attrName, attrName.c_str()) 
	   && (accessType == record.accessType))
	  break;  // FOUND
        
      }
     else   // AN ERROR FROM GET NEXT
       break; 
  }

  // CLEANUP  - END OF SCAN

  delete pscan;  

  if (status == DONE ) //  INDEX NOT FOUND -> INDEXNOTFOUND
   {
    return INDEXNOTFOUND;
   }
  else if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  return OK;
}


//-------------------------------------------------
// ADD INFO ON AN INDEX TO CATALOG
//-------------------------------------------------

Status IndexCatalog::addInfo(IndexDesc record)
{
  RID rid;

//  int len = strlen(record.relName);
//  memset(&record.relName[len], 0, sizeof record.relName - len);
//  len = strlen(record.attrName);
//  memset(&record.attrName[len], 0, sizeof record.attrName - len);
//  return insertRecord((char *)&record, sizeof(IndexDesc), rid);

  make_tuple(tuple, record);
  return insertRecord((char*)tuple, tuple->size(), rid);
}


//----------------------------------------------------
// REMOVE INFO ON AN INDEX FROM CATALOG
//-----------------------------------------------------

Status IndexCatalog::removeInfo(const std::string relation, const std::string attrName, IndexType accessType)
{
  Status status;
//  char recPtr[sizeof(IndexDesc)];
  int recSize;
  RID rid;
  Scan *pscan = NULL;
  IndexDesc record;

  // CHECK PARM

  if (relation.empty() || attrName.empty())
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::MISSING_PARAM );
 
  // OPEN SCAN

  pscan = openScan(status);

  if (!pscan)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
  if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // SCAN FILE

  while (1)
   {
//     status = pscan->getNext(rid, recPtr, recSize);
     status = pscan->getNext(rid, (char*)tuple, recSize);

     if (status == DONE  )
         break; 

     else if (status == OK)
      {
//        assert(sizeof(IndexDesc) == recSize);
//        memcpy(&record, recPtr, recSize);
	assert(tuple->size() == recSize);

	read_tuple(tuple, record);


	if(!strcasecmp (record.relName, relation.c_str()) 
	   && !strcasecmp(record.attrName, attrName.c_str())
	   && (record.accessType == accessType))
	  {
	   status = deleteRecord(rid);  //  FOUND -  DELETE	
	   break; 
	  }
      }

     else  // ERROR IN GET NEXT
        break;
  }
 
  // CLEANUP & RETURN   

  delete pscan;  

  if (status == DONE ) // NOT ALL INDEXES FOUND
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::INDEX_NOT_FOUND );
  else if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  return OK;

}



//--------------------------------------------------
// BUILD THE INDEX NAME
//-----------------------------------------------------
Status IndexCatalog::buildIndexName(const std::string relation, const std::string attrName,
			 IndexType accessType, char *&indexName)
{

  char accessName[MAXNAME];
  int sizeName;

  // ALLOCATE THE ARRAY

  indexName = new char[MAXNAME];
  if(!indexName)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );

  // DETERMINE INDEX TYPE

  if(accessType == B_Index)
    strcpy(accessName, "B_Index");
  else if(accessType == Hash)
    strcpy(accessName, "Hash");

  // CHECK FOR LEGIT NAME SIZE

  sizeName = strlen(relation.c_str()) + strlen(accessName) + strlen(attrName.c_str()) + (3 * sizeof(char));

  if(sizeName > MAXNAME)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::INDEX_NAME_TOO_LONG );

  // CREATE NAME

  strcpy(indexName, relation.c_str());
  strcat(indexName, "-");
  strcat(indexName, accessName);
  strcat(indexName, "-");
  strcat(indexName, attrName.c_str());
  return OK;
}






//--------------------------------------------------
// MAKE_TUPLE
//--------------------------------------------------
// Tuple must have been initialized properly in the 
// constructor

Status IndexCatalog::make_tuple(Tuple* tuple, IndexDesc record)
{
  tuple->setFld(1, record.relName);
  tuple->setFld(2, record.attrName);


  if (record.accessType == None)
    tuple->setFld(3, 0);
  else
    if (record.accessType == B_Index)
      tuple->setFld(3, 1);
    else
      if (record.accessType == Hash)
	tuple->setFld(3, 2);
      else
	std::cout << "Invalid accessType in IndexCatalog::make_tuple\n";

  if (record.order == Ascending)
    tuple->setFld(4, 0);
  else
    if (record.order == Descending)
      tuple->setFld(4, 1);
    else
      if (record.order == Random)
	tuple->setFld(4, 2);
      else
	std::cout << "Invalid order in IndexCatalog::make_tuple\n";

  tuple->setFld(5, record.clustered);
  tuple->setFld(6, record.distinctKeys);
  tuple->setFld(7, record.indexPages);
  
  return OK;
}




//--------------------------------------------------
// READ_TUPLE
//--------------------------------------------------

Status IndexCatalog::read_tuple(Tuple* tuple, IndexDesc& record)
{
  char* tempptr;
  tuple->getFld(1, tempptr);
  strcpy(record.relName, tempptr);

  tuple->getFld(2, tempptr);
  strcpy(record.attrName, tempptr);

  int temp;
  tuple->getFld(3, temp);
  if (temp == 0)
    record.accessType = None;
  else
    if (temp == 1)
      record.accessType = B_Index;
    else
      if (temp == 2)
	record.accessType = Hash;
      else
	  std::cout << "111Error in IndexCatalog::read_tuple\n";

  tuple->getFld(4, temp);
  if (temp == 0)
    record.order = Ascending;
  else
    if (temp == 1)
      record.order = Descending;
    else
      if (temp == 2)
	record.order = Random;
      else
	std::cout << "222Error in IndexCatalog::read_tuple\n";

  tuple->getFld(5, record.clustered);
  tuple->getFld(6, record.distinctKeys);
  tuple->getFld(7, record.indexPages);
  return OK; 
}







//----------------------------------
// DESTRUCTOR
//----------------------------------

IndexCatalog::~IndexCatalog()
{
  delete [] attrs;
  delete [] str_sizes;
  delete [] tuple;
}
