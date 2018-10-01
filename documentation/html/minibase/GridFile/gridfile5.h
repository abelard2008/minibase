// Grid File  Interface April 20, 1995 M. Stubbs  mstubbs@cs.wisc.edu
//*********************** GridFile ******************************
#ifndef GridFile_h
#define GridFile_h
#define MAX_DIMENSIONS 4
#include "/u/course/cs764-1/public/new_minirel/minirel.h"
//#include "/u/course/cs764-1/public/new_minirel/index.h"
#include "index.h"
#include "pagefile.h"
#include "diskfile.c"

class GridFile: public IndexFile 
{

friend class GridFileScan;

public:

 GridFile (enum Status &status, const char * filename,
          const void * KeyTypes, const int max_str_length, 
          const int num_dimensions);
    //Status returns the status of this operation.
    //filename is name of file.
    //num_dimensions is number of index dimensions AND types in KeyTypes
    //KeyTypes is array of AttrType where each item is the type of the
    // corresponding dimension (numbered from 0) and then the
    // array is cast to type (void *) .
    // max_str_length is the max length a string key can be
    // If GridFile index already exists, it is opened else
      //      it is created.
    // Dimensions and key types are stored in private class
    // variables for subsequent use by all GridFile procedures.


 GridFile (enum Status &status, const char * filename);
     //If GridFile index exists, opens it else error. 
    //Status returns the status of this operation.


~GridFile ();

enum Status insert (const void * KeyArray, const RID rid);
   // KeyArray is actually an array of pointers to voids the whole thing
   // cast to a void pointer. These are 
   // the keys for the respective dimensions being used. The array
   // must have an entry for each dimension 0 to 
   // num_dimensions - 1. No dimension key value should be unspecified
   // in insert.


enum Status Delete (const void * KeyArray, const RID rid);
   // KeyArray is actually an array of pointers to voids the whole thing
   // cast to a void pointer. These are 
   // the keys for the respective dimensions being used. The array
   // must have an entry for each dimension 0 to 
   // num_dimensions - 1. No dimension key value should be unspecified
   // in delete.

print ();  //print out internal variables where it makes any sense to do so

GridFileScan * new_scan (const void * LowKeys, 
                                    const void * HighKeys) ;

private:

void oldfile(Status & status, const char * filename);
void saveparameters();

int number_dimensions;
int max_string_length;
int firstpageno;
AttrType *key_types;   //pointer to array of keytypes
      //above data items are saved on first page by destructor 
char *myfilename;      // pointer to filename string
PageFile  *currfile;
Page *firstpage;
//this is temporary
diskfile *mydiskfile;
};
//***************************** Scans ****************************

class GridFileScan : public IndexFileScan
{ friend class GridFile;
 public:

enum Status get_next (RID & record_id);
    // Scan returns next record qualifies under ongoing scan if
    //        there is one.

enum Status delete_current();
    // Scan deletes current record

//private:

 GridFileScan(enum Status status, GridFile * fileptr);
   //Full scan on given gridfile.

 GridFileScan(enum Status status, GridFile * fileptr,
         const void *KeyArray);
   //Create a case of an exact scan on given gridfile.
   //If a key is not being specified here then its pointer must
   // be set to null in the KeyArray. Looks for exact match on
   // keys that are specified. Key's dimension and type must 
   // match as in insert_record and delete_record.
   
 GridFileScan(enum Status status, 
      GridFile * fileptr, const void *LowKeys, const void *HighKeys);
   //Create a case of min/max scan on given gridfile.
   //If a key is not being specified here then its pointer must
   // be set to null in the KeyArray. 
   // Key's dimension and type must 
   // match as in insert_record and delete_record.

~GridFileScan();

};
#endif 
