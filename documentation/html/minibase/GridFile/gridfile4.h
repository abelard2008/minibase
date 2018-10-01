// Grid File  Interface April 12, 1995 M. Stubbs  mstubbs@cs.wisc.edu
//*********************** GridFile ******************************
#ifndef GridFile_def__
#define GridFile_def__

#define MAX_DIMENSIONS 4
#define MAX_FILENAME 256
#include "/u/course/cs764-1/public/new_minirel/index.h"
#include "/u/course/cs764-1/public/new_minirel/minirel.h"
#include "/u/course/cs764-1/public/new_minirel/error.h"
#include "/u/course/cs764-1/public/new_minirel/error.C"

class GridFile : public IndexFile 
{

friend class GridFileScan;

public:

enum FileState {Create_failed, Not_Opened, Opened};

GridFile::GridFile (enum Status &status, const char * filename,
          const void * KeyTypes, const int max_str_length = 32, 
          const int num_dimensions = 2);
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


GridFile::GridFile (enum Status &status, char * filename);
     //If GridFile index exists, opens it else error. 
    //Status returns the status of this operation.


GridFile::~GridFile ();
      //destructor closes index

enum Status GridFile::insert (const void * KeyArray, const RID rid);
   // KeyArray is actually an array of pointers to voids the whole thing
   // cast to a void pointer. These are 
   // the keys for the respective dimensions being used. The array
   // must have an entry for each dimension 0 to 
   // num_dimensions - 1. No dimension key value should be unspecified
   // in insert.


enum Status GridFile::Delete (const void * KeyArray, const RID rid);
   // KeyArray is actually an array of pointers to voids the whole thing
   // cast to a void pointer. These are 
   // the keys for the respective dimensions being used. The array
   // must have an entry for each dimension 0 to 
   // num_dimensions - 1. No dimension key value should be unspecified
   // in delete.
   //used Delete instead of delete because compile didn't like delete,
   //probably because there is a delete operator defined  in C++.

IndexFileScan * GridFile::new_scan (const void * LowKeys, 
                                    const void * HighKeys) ;
private:
int number_dimensions;
int max_string_length;
AttrType key_types[MAX_DIMENSIONS];
char currfile[MAX_FILENAME];
enum FileState current_state;
};
//***************************** Scans ****************************

class GridFileScan : public IndexFileScan
{public:

enum Status GridFileScan:: get_next (RID & record_id);
    // Scan returns next record qualifies under ongoing scan if
    //        there is one.

enum Status GridFileScan::delete_current();
    // Scan deletes current record

private:

GridFileScan::GridFileScan(enum Status status, GridFile * fileptr);
   //Full scan on given gridfile.

GridFileScan::GridFileScan(enum Status status, GridFile * fileptr,
         const void *KeyArray);
   //Create a case of an exact scan on given gridfile.
   //If a key is not being specified here then its pointer must
   // be set to null in the KeyArray. Looks for exact match on
   // keys that are specified. Key's dimension and type must 
   // match as in insert_record and delete_record.
   
GridFileScan::GridFileScan(enum Status status, 
      GridFile * fileptr, const void *LowKeys, const void *HighKeys);
   //Create a case of min/max scan on given gridfile.
   //If a key is not being specified here then its pointer must
   // be set to null in the KeyArray. 
   // Key's dimension and type must 
   // match as in insert_record and delete_record.
   
};
#endif // GridFile_def__
