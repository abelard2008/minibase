#include "catalog.h"
#include "maincatalog.h"
extern "C" int strcasecmp( const char*, const char* );

//---------------------------------------------------
// CONSTRUCTOR
//---------------------------------------------------

AttrCatalog::AttrCatalog(Status & status):
	       HeapFile(ATTRCATNAME,status)
{
// BK
  tuple = (Tuple *) new char [Tuple::max_size];
  attrs = new AttrType[9];
  
  attrs[0] = attrString;
  attrs[1] = attrString;
  attrs[2] = attrInteger;
  attrs[3] = attrInteger;
  attrs[4] = attrInteger;  // AttrType will be represented by an integer
			   // 0 = string, 1 = real, 2 = integer
  attrs[5] = attrInteger;
  attrs[6] = attrInteger;
  attrs[7] = attrString;   // ?????  BK ?????
  attrs[8] = attrString;   // ?????  BK ?????


  // Find the largest possible tuple for values attrs[7] & attrs[8]
  //   str_sizes[2] & str_sizes[3]
  max = 10;   // comes from attrData char strVal[10]
  if (sizeof(int) > (unsigned)max)
     max = sizeof(int);
  if (sizeof(float) > (unsigned)max)
     max = sizeof(float);


  str_sizes = new short[4];
  str_sizes[0] = MAXNAME;
  str_sizes[1] = MAXNAME;
  str_sizes[2] = max;
  str_sizes[3] = max;

  tuple->setHdr(9, attrs, str_sizes);

}
 
//-----------------------------------------------------
// GET CATALOG INFO ON A SINGLE RELATION
//-----------------------------------------------------

Status AttrCatalog::getInfo(const std::string relation, const std::string attrName,
			    AttrDesc &record)
{
  Status status;
  int recSize;
  RID rid;
  Scan *pscan = NULL; 
  
  if (relation.empty() || attrName.empty())
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::MISSING_PARAM );
  
  // OPEN SCAN
  
  pscan = openScan(status);
  if (!pscan)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );

  if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
  
  // SCAN FILE FOR ATTRIBUTE
  // NOTE MUST RETURN ATTRNOTFOUND IF NOT FOUND!!!
  
  while (1)
    {
      status = pscan->getNext(rid, (char*)tuple, recSize);     // BK
      if (status == DONE  )
	{
	  delete pscan;
	  return ATTRNOTFOUND;
	}
      else if (status == OK)
        {
          assert(tuple->size() == recSize);  // BK
          read_tuple(tuple, record);
	    
          if ( !strcasecmp (record.relName, relation.c_str()) 
               && !strcasecmp(record.attrName, attrName.c_str()) )
            {
              delete pscan;
              return OK;
            }
        }
      else   // AN ERROR FROM GETNEXT
        {
          delete pscan;
          return MINIBASE_CHAIN_ERROR( CATALOG, status );
        }
    }
  return CATALOG;        // Just to make compiler happy.  
  // Should never reach this line
  //  delete pscan;  // BK <= doesn\t make any sense...
}

//---------------------------------------------------------------
// RETURNS ATTRTYPE AND STRINGSIZE ARRAYS FOR CONSTRUCTING TUPLES
// 1. AttrCatalog::getRelInfo returns attribute list IN ORDER!!!
// 2. Tuple class expects two arrays
//    1. all attibute types in positional order
//    2. an array of string sizes in positional order (NOT all attribute sizes)
//       consequently not the same size as the attribute array
//---------------------------------------------------------------

Status AttrCatalog::getTupleStructure(const std::string relation, int &attrCnt, AttrType *&typeArray, short *&sizeArray)
{
  Status  status;
  int stringcount = 0;
  AttrDesc *attrs = NULL;
  int i, x;

  // GET ALL OF THE ATTRIBUTES

  status = getRelInfo(relation, attrCnt, attrs);
  if(status != OK)
    {
      delete [] attrs;
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

  // ALLOCATE TYPEARRAY

  typeArray = new AttrType[attrCnt];
  if(!typeArray)
    {
      delete [] attrs;
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
    }

  // LOCATE STRINGS

  for(i = 0; i < attrCnt; i++)
   {
    if(attrs[i].attrType == attrString)
      stringcount++;
   }

  // ALLOCATE STRING SIZE ARRAY

  if(stringcount > 0) 
   {
     sizeArray = new short[stringcount];
     if(!sizeArray)
       {
         delete [] attrs;
         delete typeArray;
         return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
       }
   }

  // FILL ARRAYS WITH TYPE AND SIZE DATA

  for(x = 0, i = 0; i < attrCnt; i++)
   {
     typeArray[i] = attrs[i].attrType;
     if(attrs[i].attrType == attrString)
      {
	sizeArray[x] = attrs[i].attrLen;
	x++;
      }
   }

  // CLEANUP & RETURN

  delete [] attrs;
  return OK;    
}

//--------------------------------------------------
// ADD AN ENTRY TO ATTRIBUTE CATALOG
//-------------------------------------------------

Status AttrCatalog::addInfo(AttrDesc record)
{
  RID rid;
  make_tuple(tuple, record);
  return insertRecord((char*)tuple, tuple->size(), rid);    // BK
}


//-----------------------------------------------------------
// REMOVE AN ENTRY FROM ATTRIBUTE CATALOG
//----------------------------------------------------------

Status AttrCatalog::removeInfo(const std::string relation, const std::string attrName)
{
  Status status;
  int recSize;
  RID rid;
  Scan *pscan = NULL;
  AttrDesc record;
  
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
      status = pscan->getNext(rid, (char*)tuple, recSize);
      if (status == DONE)
	{
	  delete pscan;
          return MINIBASE_FIRST_ERROR( CATALOG, Catalog::ATTR_NOT_FOUND );
	}
      
      else 
	if (status == OK)
	  {
	    assert(tuple->size() == recSize);
	    read_tuple(tuple, record);
	    
	    if(!strcasecmp (record.relName, relation.c_str()) 
	       && !strcasecmp(record.attrName, attrName.c_str()))
	      {
		delete pscan;
		status = deleteRecord(rid);
		if(status != OK)
                    return MINIBASE_CHAIN_ERROR( CATALOG, status );
		return OK;
	      }
	  }
      
	else  // ERROR IN GET NEXT
	  {
	    delete pscan;
            return MINIBASE_CHAIN_ERROR( CATALOG, status );
	  }
      
    }
  
  // CLEANUP & RETURN

  delete pscan;
  return OK;
}


//------------------------------------------------------
// GET CATALOG INFO ON ALL ATTRIBUTES OF A RELATION
//------------------------------------------------------

Status AttrCatalog::getRelInfo(const std::string relation, int &attrCnt,
			       AttrDesc *&Attrs)
{
  RelDesc record;
  AttrDesc attrRec;
  Status status;
  int recSize;
  RID rid;
  Scan *pscan;
  int count = 0;


  // CHECK FOR VALID RELATION 

  if (relation.empty())
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::MISSING_PARAM );

  status = MINIBASE_RELCAT->getInfo(relation, record);
  if(status != OK)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::REL_NOT_FOUND );

  // SET ATTRIBUTE COUNT BY REFERENCE

  attrCnt = record.attrCnt;

  // OPEN SCAN

  pscan = openScan(status);
  if(! pscan)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );

  if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

   // ALLOCATE ARRAY

  if(!(Attrs = new AttrDesc[attrCnt]))
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );

  // SCAN FILE

  while(1) 
    {
      status = pscan->getNext(rid, (char*)tuple, recSize); 
      if(status != OK)
	break;
      
      assert(tuple->size() == recSize);
      read_tuple(tuple, attrRec);
      
      if(!strcasecmp(attrRec.relName, relation.c_str())) 
	{
	  memcpy(&Attrs[attrRec.attrPos - 1], &attrRec, sizeof(AttrDesc));  
	  count++;
	}
      
      if(count == attrCnt)  // if all atts found
	break; 
    }
  
   // CLEAN UP

  delete pscan;
  
  if ( status != OK && status != DONE )
    {
      delete [] Attrs;
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

  return OK;
}




//--------------------------------------------------
// MAKE_TUPLE
//--------------------------------------------------
// Tuple must have been initialized properly in the 
// constructor

Status AttrCatalog::make_tuple(Tuple* tuple, AttrDesc record)
{
  tuple->setFld(1, record.relName);
  tuple->setFld(2, record.attrName);
  tuple->setFld(3, record.attrOffset);
  tuple->setFld(4, record.attrPos);
  if (record.attrType == attrString)
    {
      tuple->setFld(5, 0);
      tuple->setFld(8, record.minVal.strVal);
      tuple->setFld(9, record.maxVal.strVal);
    }
  else
    if (record.attrType == attrReal)
      {
	tuple->setFld(5, 1);
	char* temp = new char[max]();
	memcpy(temp, &record.minVal.floatVal, sizeof(float));
	tuple->setFld(8, temp);
	memcpy(temp, &record.maxVal.floatVal, sizeof(float));
	tuple->setFld(9, temp);
      }
    else
      {
	tuple->setFld(5, 2);
	char* temp = new char[max];
	memcpy(temp, &record.minVal.intVal, sizeof(int));
	tuple->setFld(8, temp);
	memcpy(temp, &record.maxVal.intVal, sizeof(int));
	tuple->setFld(9, temp);
      }
  tuple->setFld(6, record.attrLen);
  tuple->setFld(7, record.indexCnt);
  
  return OK;
}


//--------------------------------------------------
// READ_TUPLE
//--------------------------------------------------

Status AttrCatalog::read_tuple(Tuple* tuple, AttrDesc& record)
{
  
  char* tempptr;
  tuple->getFld(1, tempptr);
  strcpy(record.relName, tempptr);
  tuple->getFld(2, tempptr);
  strcpy(record.attrName, tempptr);
  tuple->getFld(3, record.attrOffset);
  tuple->getFld(4, record.attrPos);
  int temp;
  tuple->getFld(5, temp);
  if (temp == 0)
    {
      record.attrType = attrString;
      tuple->getFld(8, tempptr);
      strcpy(record.minVal.strVal, tempptr);
      tuple->getFld(9, tempptr);
      strcpy(record.maxVal.strVal, tempptr);
    }
  else
    if (temp == 1)
      {
	record.attrType = attrReal;
	char* tempf;
	tuple->getFld(8, tempf);
	memcpy(&record.minVal.floatVal, tempf, sizeof(float));
	tuple->getFld(9, tempf);
	memcpy(&record.maxVal.floatVal, tempf, sizeof(float));
      }
    else
      if (temp == 2)
	{
	  record.attrType = attrInteger;
	  char* tempi;
	  tuple->getFld(8, tempi);
	  memcpy(&record.minVal.intVal, tempi, sizeof(int));
	  tuple->getFld(9, tempi);
	  memcpy(&record.maxVal.intVal, tempi, sizeof(int));
	}
      else
	{
	  return CATALOG;
	}
  tuple->getFld(6, record.attrLen);
  tuple->getFld(7, record.indexCnt);
  return OK; 
}



//--------------------------------------------------
// DESTRUCTOR
//--------------------------------------------------

AttrCatalog::~AttrCatalog()
{

  delete [] attrs;
  delete [] str_sizes;
  delete [] tuple;
// BK
}
