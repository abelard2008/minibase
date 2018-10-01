//----------------------------------------
// P.Feakins May 1995
// maincatalog.C
//----------------------------------------

#include "maincatalog.h"
#include "catalog.h"
#include "ext_sys_defs.h"


//--------------------------------------------------------------
// Error messages
//--------------------------------------------------------------

static const char* errorMessages[] = {
    "missing parameter",                        // MISSING_PARAM
    "insuffient memory",                        // NO_MEM
    "attribute not found",                      // ATTR_NOT_FOUND
    "relation not found",                       // REL_NOT_FOUND
    "index already exists---can't add",         // INDEX_EXISTS
    "relation name too long",                   // REL_NAME_TOO_LONG
    "relation already exists---can't create",   // REL_EXISTS
    "duplicate attributes",                     // DUP_ATTRS
    "tuple too big",                            // TUPLE_TOO_BIG
    "attribute name too long",                  // ATTR_NAME_TOO_LONG
    "index not found",                          // INDEX_NOT_FOUND
    "generated index name too long",            // INDEX_NAME_TOO_LONG
    "could not open input file",                // INPUT_OPEN
    "incorrect record size",                    // BAD_RECORD_SIZE
    "incorrect number of attributes",           // BAD_ATTR_COUNT
    "incorrect data type",                      // BAD_TYPE
};

static error_string_table errorTable( CATALOG, errorMessages );



//--------------------------------------------------------------
// CONSTRUCTOR
//-------------------------------------------------------------- 

  Catalog::Catalog(Status &status)
  {
      
      // RELCAT
      
      relCat = new RelCatalog(status);

      if(!relCat)
        {
          status = MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
          return; 
        }
      if(status != OK)
        {
          delete relCat;
          status = MINIBASE_CHAIN_ERROR( CATALOG, status );
          return;
        }
    
      // ATTRCAT

      attrCat = new AttrCatalog(status);
 
      if(!attrCat)
        {
          delete relCat;
          status = MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
          return;
        }
      if(status != OK)
        {
          delete relCat;
          delete attrCat;
          status = MINIBASE_CHAIN_ERROR( CATALOG, status );
          return;
        }

      // INDCAT

      indCat = new IndexCatalog(status);

      if(!indCat)
        {
          delete relCat;
          delete attrCat;
          status = MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
          return;
        }
      if(status != OK)
        {
          delete relCat;
          delete attrCat;
          delete indCat;
          status = MINIBASE_CHAIN_ERROR( CATALOG, status );
          return;
        }

    return ;
  }

  //----------------------------------------------------
  // DESTRUCTOR
  //----------------------------------------------------

  Catalog::~Catalog()
   {
    delete relCat;
    delete attrCat;
    delete indCat;
   }

   //----------------------------------------------------------
   // GET CATALOG ENRY FOR A RELATION
   //----------------------------------------------------------

  Status Catalog::getRelationInfo(const char* relation, RelDesc& record)
   {
       Status status=OK;

       status = relCat->getInfo(relation, record);

       if(status != OK)
           return MINIBASE_CHAIN_ERROR( CATALOG, status );

       return OK;
   }

  //-----------------------------------------------------------
  // CREATE A NEW RELATION
  //----------------------------------------------------------  

  Status Catalog::createRel(const char* relation, int attrCnt, attrInfo attrList[])
   {
       Status status;

       status = relCat->createRel( relation, attrCnt, attrList); 

       if(status != OK)
           return MINIBASE_CHAIN_ERROR( CATALOG, status );

       return OK;
   }
 
  //-------------------------------------------------------------
  // DESTROY A RELATION
  //-------------------------------------------------------------

  Status Catalog::destroyRel(const char* relation)
   {
       Status status;

       status = relCat->destroyRel( relation);

       if(status != OK)
           return MINIBASE_CHAIN_ERROR( CATALOG, status );

       return OK;
   }

  //-----------------------------------------------------------------
  // ADD AN INDEX TO A RELATION   
  //-----------------------------------------------------------------


  // I CHANGED THE buckets PARAMETER TO TAKE A DEFAULT OF ZERO
  Status Catalog::addIndex(const char* relation, const char* attrname, IndexType accessType, int buckets = 0)
   {
       Status status;

       status = relCat->addIndex(relation, attrname, accessType, buckets);
    
       if(status != OK)
           return MINIBASE_CHAIN_ERROR( CATALOG, status );

       return OK;
   }

  //-------------------------------------------------------------------- 
  // DROP INDEX FROM A RELATION 
  //--------------------------------------------------------------------

  Status Catalog::dropIndex(const char* relation, const char* attrname, IndexType accessType)
   {
       Status status;

       status = relCat->dropIndex(relation, attrname, accessType);
     
       if(status != OK)
           return MINIBASE_CHAIN_ERROR( CATALOG, status );

       return OK;
   }
 
  //-----------------------------------------------------------------------
  // GET AN ATTRIBUTE CATALOG ENTRY
  //-----------------------------------------------------------------------

  Status Catalog::getAttributeInfo(const char *relation, const char *attrName, AttrDesc &record)
   {
       Status status;

       status = attrCat->getInfo(relation, attrName, record);
     
       if(status != OK)
           return MINIBASE_CHAIN_ERROR( CATALOG, status );

       return OK;
   }
 
  //-----------------------------------------------------------------------
  // GET ALL CATALOG ATTRIBUTE ENTRIES FOR A RELATION 
  //-----------------------------------------------------------------------

  Status Catalog::getRelAttributes(const char *relation, int &attrCnt, AttrDesc *&attrs)
   {
       Status status;

       status = attrCat->getRelInfo(relation, attrCnt, attrs);

       if(status != OK)
           return MINIBASE_CHAIN_ERROR( CATALOG, status );

       return OK;
   }

  //------------------------------------------------------------------------
  // GET ALL CATALOG INDEX ENTRIES  FOR A RELATION
  //------------------------------------------------------------------------

  Status Catalog::getRelIndexes(const char* relation, int &indexCnt, IndexDesc *&indexes)
   {
       Status status;

       status = indCat->getRelInfo(relation, indexCnt, indexes);

       if(status != OK)
           return MINIBASE_CHAIN_ERROR( CATALOG, status );

       return OK;
   }

  //----------------------------------------------------------------------------
  // GET ALL CATALOG INDEX ENTRIES FOR AN ATTRIBUTE 
  //----------------------------------------------------------------------------

  Status Catalog::getAttrIndexes(const char* relation, const char* attrName, int &indexCnt, IndexDesc *&indexes)
   {
       Status status;

       status = indCat->getAttrIndexes(relation, attrName, indexCnt, indexes);

       if(status != OK)
           return MINIBASE_CHAIN_ERROR( CATALOG, status );

       return OK;
   }

  //-----------------------------------------------------------------------------
  // RETURN CATALOG ENTRY  FOR A INDEX
  //-----------------------------------------------------------------------------

  Status Catalog::getIndexInfo(const char* relation, const char* attrName, IndexType accessType,
				  IndexDesc &record)
   {
       Status status;

       status = indCat->getInfo(relation, attrName, accessType, record);

       if(status != OK)
           return MINIBASE_CHAIN_ERROR( CATALOG, status );

       return OK;
   }

 //-------------------------------------------------------------------------------
 // DUMP CONTENTS OF CATALOG TO DISK FOR OPTIMIZER
 //-------------------------------------------------------------------------------

 Status Catalog::dumpCatalog(const char* filename) 
  {
       Status status;

       status = relCat->dumpCatalog(filename);

       if(status != OK)
           return MINIBASE_CHAIN_ERROR( CATALOG, status );

       return OK;
  }

// AMIT BEGIN
 //-------------------------------------------------------------------------------
 // Run stats on the database... 
 //-------------------------------------------------------------------------------

 Status Catalog::runStats(const char* filename)
  {
       Status status;

       status = relCat->runStats(filename);

       if(status != OK)
           return MINIBASE_CHAIN_ERROR( CATALOG, status );

       return OK;
  }

// AMIT END


Status Catalog::initialize()
{
  int max;

  // *************RELCATALOG***************************************

  attrInfo* attrs;
  attrs = new attrInfo[5];
  
  attrs[0].attrType = attrString;
  strcpy( attrs[0].attrName, "relName" );
  attrs[0].attrLen = MAXNAME;      // <= NOT SAFE!!!!!

  attrs[1].attrType = attrInteger;
  strcpy( attrs[1].attrName, "attrCnt" );
  attrs[1].attrLen = sizeof(int);

  attrs[2].attrType = attrInteger;
  strcpy( attrs[2].attrName, "indexCnt" );
  attrs[2].attrLen = sizeof(int);

  attrs[3].attrType = attrInteger;
  strcpy( attrs[3].attrName, "numTuples" );
  attrs[3].attrLen = sizeof(int);

  attrs[4].attrType = attrInteger;
  strcpy( attrs[4].attrName, "numPages" );
  attrs[4].attrLen = sizeof(int);
  


  MINIBASE_CATALOGPTR->createRel(RELCATNAME, 5, attrs);

  delete [] attrs;


  // ***************  ATTRCATALOG ****************



   attrs = new attrInfo[9];


  attrs[0].attrType = attrString;
  strcpy( attrs[0].attrName, "relName" );
  attrs[0].attrLen = MAXNAME;      // <= NOT SAFE!!!!!

  attrs[1].attrType = attrString;
  strcpy( attrs[1].attrName, "attrName" );
  attrs[1].attrLen = MAXNAME;

  attrs[2].attrType = attrInteger;
  strcpy( attrs[2].attrName, "attrOffset" );
  attrs[2].attrLen = sizeof(int);

  attrs[3].attrType = attrInteger;
  strcpy( attrs[3].attrName, "attrPos" );
  attrs[3].attrLen = sizeof(int);

  // 0 = string, 1 = real, 2 = integer
  attrs[4].attrType = attrInteger;
  strcpy( attrs[4].attrName, "attrType" );
  attrs[4].attrLen = sizeof(int);


  attrs[5].attrType = attrInteger;
  strcpy( attrs[5].attrName, "attrLen" );
  attrs[5].attrLen = sizeof(int);
  
  attrs[6].attrType = attrInteger;
  strcpy( attrs[6].attrName, "indexCnt" );
  attrs[6].attrLen = sizeof(int);

  max = 10;   // comes from attrData char strVal[10]
  if (sizeof(int) > (unsigned)max)
    max = sizeof(int);
  if (sizeof(float) > (unsigned)max)
    max = sizeof(float);

  attrs[7].attrType = attrString;   // ?????  BK ?????
  strcpy( attrs[7].attrName, "minVal" );
  attrs[7].attrLen = max;

  attrs[8].attrType = attrString;   // ?????  BK ?????
  strcpy( attrs[8].attrName, "maxVal" );
  attrs[8].attrLen = max;

  MINIBASE_CATALOGPTR->createRel(ATTRCATNAME, 9, attrs);

  delete [] attrs;




  // ************* INDEXCATALOG ******************

  attrs = new attrInfo[7];
  
  attrs[0].attrType = attrString;
  strcpy( attrs[0].attrName, "relName" );
  attrs[0].attrLen = MAXNAME;      // <= NOT SAFE!!!!!

  attrs[1].attrType = attrString;
  strcpy( attrs[1].attrName, "attrName" );
  attrs[1].attrLen = MAXNAME;

  // 0 = None
  // 1 = B_Index
  // 2 = Linear Hash
  attrs[2].attrType = attrInteger;
  strcpy( attrs[2].attrName, "accessType" );
  attrs[2].attrLen = sizeof(int);


  
  // 0 = Ascending
  // 1 = Descending
  // 2 = Random
  attrs[3].attrType = attrInteger;
  strcpy( attrs[3].attrName, "order" );
  attrs[3].attrLen = sizeof(int);

  attrs[4].attrType = attrInteger;
  strcpy( attrs[4].attrName, "clustered" );
  attrs[4].attrLen = sizeof(int);
  

  attrs[5].attrType = attrInteger;
  strcpy( attrs[5].attrName, "distinctKeys" );
  attrs[5].attrLen = sizeof(int);

  attrs[6].attrType = attrInteger;
  strcpy( attrs[6].attrName, "indexPages" );
  attrs[6].attrLen = sizeof(int);

  MINIBASE_CATALOGPTR->createRel(INDEXCATNAME, 7, attrs);

  delete [] attrs;

  return OK;

}


Status Catalog::listRelations()
{
  Status status;
  Scan* relscan;

  relscan = relCat->openScan(status);

  return OK;
}





