//----------------------------------
// p. Feakins May 1995
//----------------------------------
#include "maincatalog.h"
#include "catalog.h"
#include <ostream>
#include <stdlib.h>
#include "utility.h"
// #include "tuple.h"
extern "C" int strcasecmp( const char*, const char* );



//-------------------------------
// TYPECHECK INTS
//--------------------------------

/*Checks to see if a given string is a valid int.  ["-" | ""][0..9]+  */
int check_int(attrNode N)
{
  char* index = N.attrValue;

  if (*index == '-')
      index++;
  if (!*index)
    return 0;

  while (*index)
    {
      if ((*index < '0') || (*index > '9'))
	return 0;
      index++;
    }

  return 1;
}  



//-----------------------------------
//  TYPECHECK FLOAT
//------------------------------------

/* CHecks to see if a string is a valid float. 
Nothing special.
["-" | ""] [0..9]+ ["." | ""] [0..9]+       */
int check_float(attrNode N)
{
  char* index = N.attrValue;

  if (*index == '-')
    index++;
  if (!*index)
    return 0;

  if(*index == '.')    // If we begin with a ., then we must check to make
    {                  // sure that all characters following it are numbers
      index++;
      if(!*index)
	return 0;
      else
	while(*index)
	  {
	    if ((*index < '0') || (*index > '9'))
	      return 0;
	    index++;
	  }
      return 1;
    }

  // If the first character is NOT a number (ignoring minus signs),
  // then we must check fror numbers, then check for ., then check for numbers
  while(*index && (*index != '.'))
    {
      if ((*index < '0') || (*index > '9'))
	return 0;
      index++;
    }

  if (!*index)
    return 1;
  index++;

  // We've hit a ., so we must check for only numbers now..XS
  while(*index)
    {
      if ((*index < '0') || (*index > '9'))
	return 0;
      index++;
    }
  return 1;
}


//-------------------------------------------------------------------
// CHECK STRING LENGTH
// Checks to make sure that the length of the string is within the liMits
// set by the attrDesc  
//--------------------------------------------------------------------

int check_string(attrNode /*N*/)
{

//  if (strlen(N.attrValue) > MMMMMMMMMM)
//    return FALSE;
  return 1;
}



//---------------------------------------------------
// INSERT A RECORD INTO THE DATABASE
// - WRAPS UTILITY IN A TX
//---------------------------------------------------

Status insertRecordUT(char * relation, int attrCnt, attrNode attrList[]) 
{
   Status status;


   status = insertRecUT(relation, attrCnt, attrList);

   if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

   return OK;
}


//---------------------------------------------------
// INSERT A RECORD INTO THE DATABASE
// - takes 
//   1. relation name
//   2. attribute count
//   3. array of attribute names and valuesa
// - does
//   1. typechecks
//   2. creates tuple
//   3. inserts into datafile
//   4. inserts into each indexfile
//---------------------------------------------------

Status insertRecUT(char * relation, int attrCnt, attrNode attrList[]) 
{
 RelDesc relRec;
 RID rid;
 Status status;
 AttrType attrType = attrInteger;
 int attrPos = 0;
// int attrLen;
 int attrCount;
 int indexCount;
 int recSize;
// void* data;
 char* key = 0;
 int count;
 int intVal;
 float floatVal;
 char* strVal;

 // DELETE FOLLOWING ON RETURN 
 AttrDesc *attrRecs = NULL;
 IndexDesc *indexRecs = NULL;
 Tuple* tuple = NULL;
 char *indexName = NULL;
 BTreeFile *btree = NULL;
 /*  LinearHashingFile *hash = NULL;  */
 AttrType* typeArray = NULL;
 short* sizeArray = NULL;
 HeapFile* heap = NULL;

 // GET RELATION

 status = MINIBASE_RELCAT->getInfo(relation, relRec);
 if(status != OK)  
     return MINIBASE_CHAIN_ERROR( CATALOG, status );

 // CHECK FOR VALID NO OF RECORDS

 if(relRec.attrCnt != attrCnt)
     return MINIBASE_FIRST_ERROR( CATALOG, Catalog::BAD_ATTR_COUNT );

 // GET INFO ON ATTRIBUTES

 status = MINIBASE_ATTRCAT->getRelInfo(relation, attrCount, attrRecs);
 if(status != OK)
   {
     delete [] attrRecs;
     return MINIBASE_CHAIN_ERROR( CATALOG, status );
   }

 // CHECK ATTRIBUTE LIST

 for(int z = 0; z < attrCnt; z++)
   {
    if(strcasecmp(attrRecs[z].attrName,attrList[z].attrName))
      {
        delete [] attrRecs;
        return MINIBASE_CHAIN_ERROR( CATALOG, status );
      }
   }

 // GET INFO ON INDEXES

 status = MINIBASE_INDCAT->getRelInfo(relation,indexCount, indexRecs);
 if(status != OK)
   {
     delete [] attrRecs;
     delete [] indexRecs;
     return MINIBASE_CHAIN_ERROR( CATALOG, status );
   }


 // TYPE CHECK RIGHT HERE......Make sure that the values being
 //passed are valid

 for (int i = 0; i < attrCount; i++)
  {
   switch (attrRecs[i].attrType)
   {
      case(attrInteger):
        if (!check_int(attrList[i]))
	{
	  delete [] attrRecs;
	  delete [] indexRecs;
          return MINIBASE_FIRST_ERROR( CATALOG, Catalog::BAD_TYPE );
        }
        break;
      case (attrReal):
        if (!check_float(attrList[i]))
	{
	  delete [] attrRecs;
	  delete [] indexRecs;
          return MINIBASE_FIRST_ERROR( CATALOG, Catalog::BAD_TYPE );
        }
        break;
      case (attrString):
        if (!check_string(attrList[i]))
	{
	  delete [] attrRecs;
	  delete [] indexRecs;
          return MINIBASE_FIRST_ERROR( CATALOG, Catalog::BAD_TYPE );
        }
        break;
      default:
	{
	  delete [] attrRecs;
	  delete [] indexRecs;
          return MINIBASE_FIRST_ERROR( CATALOG, Catalog::BAD_TYPE );
        }
     }
  }

   
// CREATE TUPLE  

  tuple = (Tuple *) new char[Tuple::max_size];
  if(!tuple)
    {
      delete [] indexRecs;
      delete [] attrRecs;
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

  status = MINIBASE_ATTRCAT->getTupleStructure(relation, count, typeArray,sizeArray);
  if(status != OK)
    {
      delete [] indexRecs;
      delete [] attrRecs;
      delete typeArray;
      delete sizeArray;
      delete tuple;
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

  status = tuple->setHdr(count, typeArray, sizeArray);
  if(status !=OK)
    {
      delete [] indexRecs;
      delete [] attrRecs;
      delete typeArray;
      delete sizeArray;
      delete tuple;
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

// CONVERT DATA STRINGS TO VARIABLE VALUES & INSERT INTO TUPLE

 for(int i = 0; i < relRec.attrCnt; i++)
  {
   switch (attrRecs[i].attrType)
   {
      case(attrInteger):
          intVal = atoi(attrList[i].attrValue);
	  tuple = tuple->setFld(attrRecs[i].attrPos, intVal);
          break;
      case (attrReal):
        floatVal = atof(attrList[i].attrValue);
	tuple = tuple->setFld(attrRecs[i].attrPos,floatVal );
        break;
      case (attrString):
	tuple = tuple->setFld(attrRecs[i].attrPos, attrList[i].attrValue);
        break;
      default:
	std::cerr << "Error in insertRecUT in utility.C\n";
    }
  }

 recSize = tuple->size();

// GET DATAFILE

 heap = new HeapFile(relation,status);
 if(!heap)
   {
     delete [] indexRecs;
     delete [] attrRecs;
     delete typeArray;
     delete sizeArray;
     delete tuple;
     return MINIBASE_CHAIN_ERROR( CATALOG, status );
   }

// INSERT INTO DATAFILE
 status = heap->insertRecord((char *) tuple, recSize, rid);
 if( status != OK)
   {
     delete [] indexRecs;
     delete [] attrRecs;
     delete typeArray;
     delete sizeArray;
     delete tuple;
     delete heap;
     return MINIBASE_CHAIN_ERROR( CATALOG, status );
   }

// NOW INSERT INTO EACH INDEX FOR RELATION

 for(int i=0; i < relRec.indexCnt; i++) 
   {
     status = MINIBASE_INDCAT->buildIndexName(relation, indexRecs[i].attrName, //  
			   indexRecs[i].accessType, indexName);
     if(status != OK)
        break;

     // FIND INDEXED ATTRIBUTE

     for(int x = 0; x < attrCnt ; x++)
       {
          if(!strcasecmp(attrRecs[x].attrName,indexRecs[i].attrName))
            {
               attrType = attrRecs[x].attrType;
	       attrPos  = attrRecs[x].attrPos;
	       break;
            }
        }
 
     // PULL OUT KEY

     switch(attrType)
      {
        case attrInteger  : intVal = tuple->getFld(attrPos, intVal);
			    key = (char *) &intVal;  
                            break;

        case attrReal     : floatVal = tuple->getFld(attrPos, floatVal);
			    key = (char *) &floatVal;  
                            break;

	case attrString   : strVal = tuple->getFld(attrPos, strVal);
			    key = strVal;    
                            break;
        default:
	  std::cerr << "Error in insertRecUT\n";
       }
   

   // INSERT INTO INDEX

   // BTREE INSERT CODE

     if(indexRecs[i].accessType == B_Index)
        {
           btree = new BTreeFile(status,indexName);
           if(!btree) 
	     {
	       status = INSUFMEM;
	       break;
             }
           if(status != OK)
	      break;

           status = btree->insert(key,rid);
           if(status != OK)
	      break; 

           delete btree;
	   btree = NULL;
        } 

   // HASH INSERT CODE 

/*
     else if(indexRecs[i].accessType == Hash)
       {
          hash = new LinearHashingFile(status,indexName);
          if(!hash) 
	    {
	       status = INSUFMEM;
	       break;
            }
          if(status != OK)
	     break;

          status = hash->insert(key,rid);
          if(status != OK)
	     break;

          delete hash;
	  hash = NULL;
    } 
*/
 } // end for loop - errors break out of loop

 // CLEANUP

  delete heap;
  delete [] attrRecs;
  delete [] indexRecs;
  delete indexName;
  indexName = NULL;
  delete typeArray;
  delete sizeArray;
  delete tuple;
  if(status == INSUFMEM)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
  else if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  return OK;
}




//-------------------------------------------------------------
// WRAPS DELETE RECORD UTILITY IN A TX
//--------------------------------------------------------------

Status deleteRecordUT(char * relation, const attrNode node) 

{
   Status status;

   status = deleteRecUT(relation, node);

   if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

   return OK;
}



//--------------------------------------------------------------
// DELETE RECORD(S) BASED ON AN = PREDICATE ON SOME FIELDa
//---------------------------------------------------------------

Status deleteRecUT(char* relation, const attrNode node)
{
  Status  status;
  RelDesc relRec;
  AttrDesc attrRec;
//  int indexCount;
  int recSize;
  int count;
  int intVal;
  int intVal2;
  float floatVal;
  float floatVal2;
  char * strVal2;
  int sameKey = false;
  char *key;
  
  RID MINIBASE_BADRID;
  MINIBASE_BADRID.pageNo = -1;
  MINIBASE_BADRID.slotNo = -1;

  RID old_rid = MINIBASE_BADRID;
  RID new_rid = MINIBASE_BADRID;

  // DELETE BEFORE RETURN
  AttrType *typeArray = NULL;
  short *sizeArray = NULL;
  Tuple *tuple = NULL;
  HeapFile *heap = NULL;
  Scan *pscan = NULL;


  // GET RELATION INFO

  status = MINIBASE_RELCAT->getInfo(relation, relRec);
  if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  // GET ATTRIBUTE INFO

  status = MINIBASE_ATTRCAT->getInfo(relation, node.attrName, attrRec);
  if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );


  // TYPE CHECKING  

  switch (attrRec.attrType)
   {
     case attrInteger :
        if (!check_int(node))
            return MINIBASE_FIRST_ERROR( CATALOG, Catalog::BAD_TYPE );
        intVal = atoi(node.attrValue);
	key = (char *) &intVal;
//	int num = *((int *)key);
	break;
     case attrReal :
        if (!check_float(node))
            return MINIBASE_FIRST_ERROR( CATALOG, Catalog::BAD_TYPE );
        floatVal = atof(node.attrValue);
	key = (char *) &floatVal;
	break;
     case attrString :
        if (!check_string(node))
            return MINIBASE_FIRST_ERROR( CATALOG, Catalog::BAD_TYPE );
	key = node.attrValue;
	break;
     default:
	std::cerr << "Error in deleteRecUT\n";
    }
	

  // GET HEAP FILE

  heap = new HeapFile(relation, status);
  if(!heap)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );

  if(status != OK)
    {
      delete heap;
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

  // OPEN SCAN

  pscan = heap->openScan(status);
  if(!pscan)
   {
     delete heap;
     return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
   }
  if(status != OK)
    {
      delete heap;
      delete pscan;
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

// INITIALIZE TUPLE

 tuple = (Tuple *) new char[Tuple::max_size];
 if(!tuple)
   {
     delete heap;
     delete pscan;
     return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
   }

 status = MINIBASE_ATTRCAT->getTupleStructure(relation, count, typeArray, sizeArray);
 if(status != OK)
   {
     delete heap;
     delete pscan;
     delete sizeArray;
     delete typeArray;
     delete tuple;
     return MINIBASE_CHAIN_ERROR( CATALOG, status );
   }

 status = tuple->setHdr(count, typeArray, sizeArray);
 if(status != OK)
   {
     delete heap;
     delete pscan;
     delete sizeArray;
     delete typeArray;
     delete tuple;
     return MINIBASE_CHAIN_ERROR( CATALOG, status );
   }


//  SCAN FILE FOR MATCHING RECORDS

  while (1)
   {
     status = pscan->getNext(new_rid, (char *) tuple, recSize);
     if(status == DONE )
      {
       status = OK;
       break;
      }
     else if(status != OK)
       break;

 // pull out the key and compare...

     switch(attrRec.attrType)
      {
        case attrInteger  : intVal2 = tuple->getFld(attrRec.attrPos, intVal2);
			    if(intVal == intVal2)
			      sameKey = 1; 
                            else          
			      sameKey = 0;
                            break;

        case attrReal     : floatVal2 = tuple->getFld(attrRec.attrPos, floatVal);
			    if(floatVal == floatVal2)
			      sameKey = 1; 
                            else          
			      sameKey = 0;
                            break;

	case attrString   : strVal2 = tuple->getFld(attrRec.attrPos, strVal2);
			    if(!strcasecmp(node.attrValue ,strVal2))
			      sameKey = 1;
                            else
			      sameKey = 0;
                            break;
        default:
	  std::cerr << "Error in deleteRecUT in utility.C\n";
       }

   // IF TRUE PERFORM DELETE INDEXES AND DLETE HEAPFILE

     if(sameKey == 1)
      {
        std::cout << "deleting record" << std::endl;
       status = deleteRecIndexesUT(relation, new_rid, tuple);
       if(status != OK)
	  break;

       if(old_rid != MINIBASE_BADRID) {
          status = heap->deleteRecord(old_rid);
          if(status != OK)
	    break;
       }
      old_rid = new_rid;
      }
    }  // end while loop

    if(old_rid != MINIBASE_BADRID) {
       status = heap->deleteRecord(old_rid);
    }
// CLEANUP POINTERS

  delete pscan;
  delete heap;
  delete typeArray;
  delete sizeArray;
  delete tuple;
  if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

  return OK;
}
	


//---------------------------------------------------------------
// DELETE INDEXES FOR KEY,RID PAIRS
//---------------------------------------------------------------

Status deleteRecIndexesUT(char* relation, RID rid, Tuple* &tuple)
{
  Status status;
  BTreeFile *btree = NULL;
  /*  LinearHashingFile *hash = NULL;  */
  char *indexName = NULL;
  IndexDesc *indRecs = NULL;
  AttrDesc attrRec ;
  int indexCount;
  int intVal;
  float floatVal;
  char *key;

  // GET INFO ON INDEXES

  status = MINIBASE_INDCAT->getRelInfo(relation, indexCount, indRecs);
  if(status != OK)
    {               
      delete [] indRecs;
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

  // IF NO INDEXES JUST RETURN

  if(indexCount == 0)
   {
    delete [] indRecs;
    return OK;
   }

  // FOR EACH INDEX DELETE INDEX RECORD 

  for(int i = 0; i < indexCount; i++)
    {
     // GET ATTRIBUTE INFO

     status = MINIBASE_ATTRCAT->getInfo(relation, indRecs[i].attrName, attrRec);
     if(status != OK)
       {
	 delete [] indRecs;
         return MINIBASE_CHAIN_ERROR( CATALOG, status );
       }

     // BUILD INDEX NAME

     status = MINIBASE_INDCAT->buildIndexName(relation,indRecs[i].attrName,indRecs[i].accessType,
				     indexName);

     if(status != OK)
       {
         delete indexName;
	 indexName = NULL;
         delete [] indRecs;
         return MINIBASE_CHAIN_ERROR( CATALOG, status );
       }

     // EXTRACT KEY FROM TUPLE 

     switch (attrRec.attrType)
       {
        case attrInteger  : intVal = tuple->getFld(attrRec.attrPos, intVal);
			    key = (char *) &intVal;  
                            break;

        case attrReal     : floatVal = tuple->getFld(attrRec.attrPos, floatVal);
			    key = (char *) &floatVal;  
                            break;

	case attrString   : key = tuple->getFld(attrRec.attrPos, key);
                            break;
        default:
	  std::cerr << "Error in deleteRecIndexesUT\n";
        }

     // HASH DELETE 


/*
     if(indRecs[i].accessType == Hash)
       {
          hash = new LinearHashingFile(status, indexName);
          if(!hash) 
	   {
	     status = INSUFMEM ; 
	     break;
           }
	  if(status != OK)
             break;

          status = hash->Delete(key, rid);
          if(status != OK)
              break; 

        } // END HASH
     else
*/

    // BTREE DELETE 

     if(indRecs[i].accessType == B_Index)
         {
	   btree = new BTreeFile(status, indexName);
	   if(!btree)
	     {
	      status = INSUFMEM;
	      break;
             }
	   if(status != OK)
	      break;

           status = btree->Delete(key, rid);
           if(status != OK)
	      break; 

          }   // END BTREEA

     if(btree)
       {
	 delete btree;
	 btree = NULL;
       }
/*
     if(hash)
       {
	 delete hash;
	 hash = NULL;
       }
*/
     delete indexName;
     indexName = NULL;

    } // end for loop

  // CLEANUP SECTION

  if (indexName)
     {
       delete indexName;
       indexName = NULL;
     }
  if(btree)
    {
      delete btree;
      btree = NULL;
    }
/*
  if(hash)
    {
      delete hash;
      hash = NULL;
    }
*/
  delete [] indRecs;

  // CHECK ERROR STATE

  if(status == INSUFMEM)
      return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
  else if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );
  else
     return OK;
 }
     
       

