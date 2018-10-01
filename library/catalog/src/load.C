//------------------------------------
// P. Feakins May 1995
//-----------------------------------
 
#include <ostream>
#include <fstream>
#include <string.h>
#include <string>
#include "maincatalog.h"
#include "catalog.h"
#include "utility.h"
extern "C" int strcasecmp( const char*, const char* );

//Status loadIndexesUT(Tuple *&tuple, const int attrCnt, const int indexCnt, const AttrDesc *&attrs, const IndexDesc *&indexes, void **&iFiles, const RID &rid );

//--------------------------------------------------------------------------------
// MAIN LOAD UTILITY
//-------------------------------------------------------------------------------- 

Status loadUT(char *relation, char *filename)
{
   Status status = loadRecordsUT(relation, filename);

   if(status != OK)
      return MINIBASE_CHAIN_ERROR( CATALOG, status );

   return OK;
}

//---------------------------------------------------------------------------------
// LOADS RECORDS INTO THE DATABASE
// 1. CALLS CONSTRUCTOR FOR DATAFILE & ALL INDEX FILES
//    - maintains a list of pointers to index names
//    - maintains a list of pointers to index files
//      - indexfiles are 'opened' once
// 2. INSERTS RECORDS INTO HEAPFILE
// 3. CALLS loadRecIndexes TO LOAD EACH INDEX
//    - note that a pointer to an index file (iFiles[i]) is passed
//      to loadIndexRecs
//    - for each index the pointer from iFiles is passed
//---------------------------------------------------------------------------------

Status loadRecordsUT(char* relation, char* filename)
{
 RelDesc relRec;
 Status status;
 RID rid;
 int attrCnt;  // no of atttributes
 int indexCnt; // no of indexes
 int  typecount;   // count of the type array
 std::fstream infile;
 int tupleSize;
 int intVal;
 float floatVal;
 char stringVal[255];
 int i,x; // xid;
 // DELETE FOLLOWING BEFORE RETURN
 HeapFile *heap = 0;
 Tuple *tuple = 0;
 AttrType *typeArray = 0;
 AttrDesc *attrs = 0;   
 IndexDesc *indexes = 0;
 // IndexFile **iFiles = 0;
 void **iFiles = 0;
 char **iNames = 0;
 short *sizeArray = 0;

 status = MINIBASE_RELCAT->getInfo(relation, relRec); 
 if(status != OK)
     return MINIBASE_CHAIN_ERROR( CATALOG, status );


 // GET ATTRIBUTE INFO

 status = MINIBASE_ATTRCAT->getRelInfo(relation, attrCnt, attrs);
 if(status != OK)
   {
     delete [] attrs;
     return MINIBASE_CHAIN_ERROR( CATALOG, status );
   }


 // GET INDEX INFO

 status = MINIBASE_INDCAT->getRelInfo(relation, indexCnt, indexes);
 if(status != OK)
   {
     delete [] attrs;
     delete [] indexes;
     return MINIBASE_CHAIN_ERROR( CATALOG, status );
   }


 // SET UP TUPLE

 tuple = (Tuple *) new char[Tuple::max_size];
 if(!tuple)
   {
     delete [] attrs;
     delete [] indexes;
     return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
   }

 status = MINIBASE_ATTRCAT->getTupleStructure(relation, typecount, typeArray, sizeArray);
 if(status != OK)
   {
     delete [] attrs;
     delete [] indexes;
     delete tuple;
     delete typeArray;
     delete sizeArray;
     return MINIBASE_CHAIN_ERROR( CATALOG, status );
   }
 
 status = tuple->setHdr(typecount, typeArray, sizeArray);
 if(status != OK)
   {
     delete [] attrs;
     delete [] indexes;
     delete tuple;
     delete typeArray;
     delete sizeArray;
     return MINIBASE_CHAIN_ERROR( CATALOG, status );
   }

 tupleSize = tuple->size();


 // CREATE INDEX SECTIOM

 // CREATE ARRAY OF NAME POINTERS

 iNames = new char* [indexCnt];
 if(!iNames)
   {
     delete [] attrs;
     delete [] indexes;
     delete tuple;
     delete typeArray;
     delete sizeArray;
     return MINIBASE_CHAIN_ERROR( CATALOG, status );
   }

 // INITIALIZE POINTERS TO 0

 for(i = 0; i < indexCnt; i++)
  {
   iNames[i] = 0;
  }

 // BUILD INDEX NAMES

 for(i = 0; i < indexCnt; i++)
  {
    status = MINIBASE_INDCAT->buildIndexName( relation, indexes[i].attrName,
                                              indexes[i].accessType, iNames[i]);
   
   if(status != OK)
     {
       delete [] attrs;
       delete [] indexes;
       delete tuple;
       delete typeArray;
       delete sizeArray;
       for(x = 0; x < indexCnt; x++)
           delete iNames[x];
       delete [] iNames;
       return MINIBASE_CHAIN_ERROR( CATALOG, status );
     }
  }   // END BUILD NAMES



 // CREATE ARRAY OF INDEX FILE PTRS

 iFiles = new void* [indexCnt];
 if(!iFiles)
   {
     delete [] attrs;
     delete [] indexes;
     delete tuple;
     delete typeArray;
     delete sizeArray;
     for(x = 0; x < indexCnt; x++)
         delete iNames[x];
     delete [] iNames;
     return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
   }
  
 // INITIALIZE POINTERS TO 0

 for(i = 0; i < indexCnt ; i++)
  {
   iFiles[i] = 0;
  }

 // NEW EACH INDEX (BTREE & HASH) AND ASSIGN TO POINTERS

 for(i = 0; i < indexCnt; i++)
  {
   switch(indexes[i].accessType)
    {
      case  B_Index : iFiles[i] =  new BTreeFile(status, iNames[i]);
             	      if(!iFiles[i])  // ERROR
                        {
                          delete [] attrs;
                          delete [] indexes;
                          delete typeArray;
                          delete sizeArray;
                          delete tuple;
                          for(x = 0; x < indexCnt; x++)
		            {
			      delete iNames[x];
			      delete [] iFiles[x];
                            }
                          delete [] iNames;
                          delete [] iFiles;
                          return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
                        }
		      break;

      case  Hash:
/*
              iFiles[i] = new LinearHashingFile(status, iNames[i]);
		      if(!iFiles[i]) // ERROR
                        {
                          delete [] attrs;
                          delete [] indexes;
                          delete typeArray;
                          delete sizeArray;
                          delete tuple;
                          for(x = 0; x < indexCnt; x++)
			    {
			      delete iNames[x];
			      delete [] iFiles[x];
                            }
                          delete [] iNames;
                          delete [] iFiles;
                          return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );
                }
		       break;
        */
      case None:
          break;
    } // end switch
  }



 // OPEN UP HEAP FILE

 heap = new HeapFile(relation, status);

 if(status != OK || !heap)
  {
    delete [] attrs;
    delete [] indexes;
    delete tuple;
    delete typeArray;
    delete sizeArray;
    for(x = 0; x < indexCnt; x++)
     {
      delete iNames[x];
      delete [] iFiles[x];
     }
    delete [] iNames;
    delete [] iFiles;
    
    if(!heap)
        return MINIBASE_FIRST_ERROR( CATALOG, Catalog::NO_MEM );

    delete heap;
    return MINIBASE_CHAIN_ERROR( CATALOG, status );
  }

 

 // OPEN INPUT FILE 

 infile.open(filename, std::ios::in);
 if(!infile)
     status = MINIBASE_FIRST_ERROR( CATALOG, Catalog::INPUT_OPEN );

 // PROCESS RECORDS FROM INPUT FILE

 while(status == OK)
  {
   
   for(i = 0; i < attrCnt; i++)
    {
     switch(attrs[i].attrType)     //CHECK ATTRIBUTE TYPE & BUILD TUPLE
      {
       case attrInteger  : infile >> intVal; 
			   tuple = tuple->setFld(attrs[i].attrPos,intVal); 
			   //std::cout << intVal << endl;
			   break;

       case attrReal     : infile >> floatVal; 
			   tuple = tuple->setFld(attrs[i].attrPos,floatVal); 
			   //std::cout << floatVal << endl;
			   break;

       case attrString   : infile >> stringVal;
			   tuple = tuple->setFld(attrs[i].attrPos,stringVal);
			   //std::cout << stringVal << endl;
			   break;

       case attrNull: case attrSymbol: break;
      } 
    } 
   
   // INSERT TUPLE INTO HEAP FILE

   //tuple->print(typeArray);
   status = heap->insertRecord((char *) tuple, tupleSize, rid);  
   if(status != OK)
    {
     break;
    }

   // INSERT INTO INDEX FILES

   if(indexCnt > 0)
     status = loadIndexesUT(tuple, attrCnt, indexCnt, attrs, indexes, iFiles, rid);
   if(status != OK)
     break;

   // GET NEXT RECORD / CHECK FOR EOF

   infile >> std::ws;
   
   if(infile.eof())
    {
      status = DONE;
      break;
    }

  }  // ends while (status == OK)

 // CLEAN UP POINTERS & FILE

  

 delete [] attrs;
 delete [] indexes;
 delete tuple;
 delete typeArray;
 delete sizeArray;
 delete heap;
 for(x = 0; x < indexCnt; x++)
   {
      delete iNames[x];
      delete  iFiles[x];
   }
 delete [] iFiles;
 delete [] iNames;

 infile.close();

 if(status != DONE)
     return MINIBASE_CHAIN_ERROR( CATALOG, status );

 return OK;

}


//-----------------------------------------------------------------------------
// INSERTS RECORDS INTO INDEXES
// - does for each index
//   1. finds appropriate key in tuple
//   2. inserts into appropriate index using iFiles pointers
//-----------------------------------------------------------------------------

Status loadIndexesUT(Tuple *&tuple, const int attrCnt, const int indexCnt,
      AttrDesc *&attrs,  IndexDesc *&indexes, void **&iFiles,  RID &rid )
{
  Status status = OK;
  char *key ;
  int intVal;
  float floatVal;
  AttrType attrType = attrInteger;
  int attrPos = 0;
	
  // FOR EACH INDEX

  for(int i = 0; i < indexCnt; i++)
   {
      for(int x = 0; x < attrCnt; x++)
       {
	 // LOCATE ATTRIBUTE INFO FOR CURRENT INDEX

	 if(!strcasecmp(indexes[i].attrName, attrs[x].attrName))
	  {
	    attrType = attrs[x].attrType;
	    attrPos = attrs[x].attrPos;
            break;
          }
       } 

      // OBTAIN INDEX KEY

      switch(attrType)
       {
	 case attrInteger  : intVal = tuple->getFld(attrPos, intVal);
		       	     key = (char *) &intVal;
			     break;

	 case attrReal     : floatVal = tuple->getFld(attrPos, floatVal);
			     key = (char *) &floatVal;
			     break;

	 case attrString   : key = tuple->getFld(attrPos, key);
			    break;

	 case attrNull: case attrSymbol: break;
       }

      // INSERT KEY, RID INTO INDEX

/*
      if(indexes[i].accessType == Hash)
         status = ((LinearHashingFile *)iFiles[i])->insert(key, rid);
      else
*/
      if(indexes[i].accessType == B_Index)
         status = ((BTreeFile *)iFiles[i])->insert(key, rid);

      if(status != OK)
          return MINIBASE_CHAIN_ERROR( CATALOG, status );
    }

  // CLEAN UP
  
  return OK;
}



