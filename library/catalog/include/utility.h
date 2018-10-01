//-----------------------------------
// utility header file
//----------------------------------
#include "buf.h"

extern BufMgr *bm;

typedef struct
 {
  char * attrName;
  char * attrValue;
 } attrNode;

 // WRAPS DELETE UTILITY IN TX
 Status deleteRecordUT(char *relation, const attrNode item);

 // DELETES RECORDS
 Status deleteRecUT(char *relation, const attrNode item);

 // DELETES INDEX ENRIES FOR RECORDS
 Status deleteRecIndexesUT(char* relation, RID rid, Tuple* &tuple);

 // WRAPS INSERT UTILITY  IN TX
 Status insertRecordUT(char *relation, int attrCnt, attrNode attrList[]);

 // INSERTS RECORDS
 Status insertRecUT(char *relation, int attrCnt, attrNode attrList[]);

 // WRAPS LOAD UTILITY IN TX
 Status loadUT(char *relation, char *fileName);

 // LOADS RECORDS
 Status loadRecordsUT(char *relation, char *fileName);

 // LOADS INDEXES
 Status loadIndexesUT(Tuple *&tuple, const int attrCnt, const int indexCnt,
     AttrDesc *&attrs, IndexDesc *&indexes, void **&iFiles, RID &rid );

