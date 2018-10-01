#ifndef CATALOG_H
#define CATALOG_H

#include "scan.h"
//#include "error.h"
#include "heapfile.h"
#include "index.h"
#include <string.h>
#include <strings.h>
#include <string>
#include <assert.h>
#include "minirel.h"
#include "scan.h"
#include <ostream>
#include <fstream>
#include <ostream>
#include "tuple.h"
#include "new_error.h"
#include "btree.h"
#include "linearhashing.h" 

// define if debug output wanted
//#define DEBUGCAT

// forward declarations
class Catalog;
class AttrCatalog;
class IndexCatalog;

#define RELCATNAME   "relcat"           // name of relation catalog
#define ATTRCATNAME  "attrcat"          // name of attribute catalog
#define INDEXCATNAME "indcat"
#define RELNAME      "relname"          // name of indexed field in rel/attrcat
#define MAXNAME      32                 // length of relName, attrName
#define MAXSTRINGLEN 255                // max. length of string attribute
#define NUMTUPLESFILE  2000             // default statistic = no recs in file
#define NUMPAGESFILE     50             // default statistic = no pages in file
#define DISTINCTKEYS   2000             // default statistic: no of distinct keys
#define INDEXPAGES       10             // default statisitc no of index pages
#define MINSTRINGVAL "A"                // default statisitic
#define MAXSTRINGVAL "ZZZZZZZ"          // default statisitic
#define  MINNUMVAL   0                    // default statistic
#define  MAXNUMVAL   999999999            // default statisitic

// attrData struct for minimum and maximum attribute values

typedef struct {
  char strVal[10];
  int intVal;
  float floatVal ;
} attrData;


//   RelDesc struct: schema of relation catalog:


typedef struct {
  char relName[MAXNAME];                // relation name
  int attrCnt;                          // number of attributes
  int indexCnt;                         // number of indexed attrs
  int numTuples;                        // number of tuples in the relation
  int numPages;                         // number of pages in the file
} RelDesc;


// attrInfo struct used for creating relations

typedef struct {
  char attrName[MAXNAME];               // attribute name
  AttrType attrType;                    // INTEGER, FLOAT, or STRING
  int attrLen;                          // length
} attrInfo; 


// AttrDesc struct : schema of attribute catalog:


typedef struct {
  char relName[MAXNAME];                // relation name
  char attrName[MAXNAME];               // attribute name
  int attrOffset;                       // attribute offset
  int attrPos;                          // attribute position 
  AttrType attrType;                   // attribute type
  int attrLen;                          // attribute length
  int indexCnt;                        // number of indexes
  attrData minVal;                     // min max key values
  attrData maxVal;

} AttrDesc;

// IndexDesc Struct : schema for index catalog  

typedef struct {
  char relName[MAXNAME];                // relation name
  char attrName[MAXNAME];               // attribute name
  IndexType accessType;                 // access method 
  TupleOrder order;                     // order of keys   
  int clustered;                        //  
  int distinctKeys;                     // no of distinct key values 
  int indexPages;                       // no of index pages 

} IndexDesc;

class RelCatalog : public HeapFile {
 friend Catalog;
 friend AttrCatalog;
 friend IndexCatalog;

 public:
  // CONSTRUCTOR
  RelCatalog(Status &status);

  // GET RELATION DESCRIPTION FOR A RELATION
  Status getInfo(char* relation, RelDesc& record);

  // DESTRUCTOR
  ~RelCatalog();

private:
  // CREATE A NEW RELATION
  Status createRel(char* relation, int attrCnt, attrInfo attrList[]);

  // DESTROY A RELATION
  Status destroyRel(char* relation);

  // ADD AN INDEX TO A RELATION
  Status addIndex(char* relation, char* attrname, IndexType accessType, int buckets = 1);

  // DROP AN INDEX FROM A RELATION
  Status dropIndex(char* relation, char* attrname, IndexType accessType);

  // DUMPS A CATALOG TO A DISK FILE (FOR OPTIMIZER)
  Status dumpCatalog();

  // OUTPUTS A RELATION TO DISK FOR OPTIMIZER
  Status dumpRelation(std::fstream &outFile, RelDesc &relRec, int tupleSize); 

  // OUTPUTS ATTRIBUTES TO DISK FOR OPTIMIZER
  Status dumpRelAttributes (std::fstream &outFile,AttrDesc *&attrRecs, int attrCnt);

  // OUTPUTS ACCESS METHODS TO DISK FOR OPTIMIZER
  Status dumpRelIndex(std::fstream &outFile,IndexDesc *&indexRecs, int indexCnt);

  // ADD INFORMATION ON A RELATION TO  CATALOG
  Status addInfo(RelDesc record);

  // REMOVE INFORMATION ON A RELATION FROM CATALOG
  Status removeInfo(char *relation);
};



class AttrCatalog : public HeapFile {
 friend Catalog;
 friend RelCatalog;
 friend IndexCatalog;

 public:
  // OPEN ATTRIBUTE CATALOG
  AttrCatalog(Status &status);

  // GET ATTRIBUTE DESCRIPTION
  Status getInfo(char *relation, char *attrName, AttrDesc &record);

  // GET ALL ATTRIBUTES OF A RELATION
  Status getRelInfo(char *relation, int &attrCnt, AttrDesc *&attrs);

  // RETURNS ATTRTYPE AND STRINGSIZE ARRAYS FOR CONSTRUCTING TUPLES
  Status getTupleStructure(char * relation, int &attrCnt, AttrType *&typeArray, short *&sizeArray);
  
  // CLOSE ATTRIBUTE CATALOG
  ~AttrCatalog();
  
 private:
  // ADD ATTRIBUTE ENTRY TO CATALOG
  Status addInfo(AttrDesc record);

  // REMOVE AN ATTRIBUTE ENTRY FROM CATALOG
  Status removeInfo(char *relation, char *attrName);

  // REMOVE ALL ATTRIBUTE ENTRIES FOR A RELATION
  Status dropRelation(char* relation);

  // ADD AN INDEX TO A RELATION
  Status addIndex(char* relation, char* attrname, IndexType accessType);


};


class IndexCatalog : public HeapFile {
 friend Catalog;
 friend RelCatalog;
 friend AttrCatalog;

 public:
 // OPEN INDEX CATALOG
 IndexCatalog(Status &status);

 // GET ALL INDEXES FOR A RELATION
 Status getRelInfo(char* relation, int &indexCnt, IndexDesc *&indexes);

 // RETURN INFO ON AN INDEX
 Status getInfo(char* relation, char* attrName, IndexType accessType, IndexDesc &record);

 // GET ALL INDEXES INLUDING A SPECIFIED ATTRIBUTE
 Status IndexCatalog::getAttrIndexes(char* relation, char *attrName, int &indexCnt, IndexDesc *&indexes);

 // CREATES A FILE NAME FOR AN INDEX 
 Status buildIndexName(char* relation, char* attrName, IndexType accessType, char  *&indexName);

 // DESTRUCTOR
 ~IndexCatalog();

 private:
 // ADD INDEX ENTRY TO CATALOG
 Status addInfo(IndexDesc record);

 // REMOVE INDEX ENTRY FROM CATALOG
 Status removeInfo(char* relation, char* attrName, IndexType accessType);

 // ADD INDEX TO A RELATION
 Status addIndex(char* relation, char* attrName, IndexType accessType, int buckets = 1);

 // DROP INDEX FROM A RELATION
 Status dropIndex(char* relation, char* attrName, IndexType accessType);

 // DROP ALL INDEXES FOR A RELATION
 Status IndexCatalog::dropRelation(char *relation);


};

extern IndexCatalog  *indCat;
extern RelCatalog  *relCat;
extern AttrCatalog *attrCat;
extern global_error *minirel_error;
// extern DB *database;
//extern Error error;

#endif
