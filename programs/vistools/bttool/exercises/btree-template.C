/* This is the template C++ file for the B+ tree exercises.  You copy
   this file to a different name, change it to insert and delete
   different keys from the tree, and then compile it with the
   following command:

      make NAME=your-file-name

   (where the name you copied this file to is "your-file-name.C").

   You can ignore most of the setup code in this file.  You should
   start reading where it says BEGIN READING HERE.

*/


#define BT_DEBUG
#define BT_TRACE

#include <ostream>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "mr_syserr.h"
#include "minirel.h"
#include "new_error.h"
#include "db.h"
#include "buf.h"
#include "btfile.h"
#include "hfpage.h"
#include "catalog.h"
#include "maincatalog.h"

#include <ext_sym_tabs.h>
#include <ext_token.h>
#include <parse_func_defs.h>
#include <globals.h>
#include <mr_syserr.h>

#include <assert.h>
#include "menu.h"


/* include headers for multi-user minibase*/
#include "log.h"
#include "os.h"
#include "xaction_mgr.h"
#include "recovery_mgr.h"

global_error* minirel_error;
SystemDefs*  minibase_globals;

AttrCatalog* attrCat;
RelCatalog* relCat;
IndexCatalog* indCat;

void init_minirel_error();
void gen_rec(void* rec, int key, int len);
void gen_rec(void* rec, char *key, int len);

void init_minirel_error()
{
    minirel_error = new global_error;
    return;
}

//generating record based on integer key
void gen_rec(void* rec, int key, int len)
{
    // heck to make sure that the len of the record is preserved
    memset(rec, ' ', len-1);  
    ((char*)rec)[len-2] = '0';
    ((char*)rec)[len-1] = '\0';
    char tmp[100];
    sprintf(tmp, "record #%04d ", key);
    memcpy((char*)rec, tmp, strlen(tmp));
}

//generating record based on string key
void gen_rec(void* rec, char *key, int len)
{
    // heck to make sure that the len of the record is preserved
    memset(rec, ' ', len-1);  
    ((char*)rec)[len-2] = '0';
    ((char*)rec)[len-1] = '\0';
    memcpy((char*)rec, key, strlen(key));
}

int main()
{
    init_minirel_error();
    MINIBASE_CurrXid =0;

    Status status;
    system("/bin/rm -rf /tmp/btlog /tmp/BTDRIVER");
    minibase_globals = new SystemDefs("/tmp/BTDRIVER", "/tmp/btlog",
                                      1000,500,1);

    // BEGIN READING HERE:

    // Now we create the b-tree "file" within the database.  This example
    // sets the string length to 220, which in our 1K-page system equates
    // to 4 records per page.

    // there are two options for creating a btree

    // one option is the delete algorithm to use - FULL_DELETE or NAIVE_DELETE
    // Choose the delete algorithm you want to use by commenting out one
    // of the following two btree construtor statements. 

    // using full_delete algorithm
    BTreeFile *tree = new BTreeFile(status, "btree", attrString, 220, FULL_DELETE);

    // using naive delete algorithm
    // BTreeFile *tree = new BTreeFile(status, "btree", attrString, 220, NAIVE_DELETE);

    // the other option is the output file for trace.
    // if no argument is given for output stream, the default is stdout (std::cout).
    
    // the following will create a btree with trace written to file "btree_trace"
//     ofstream * trace = new ofstream("btree_trace");
//     if (!trace) {
//         interp->result = "error: cannot open file btree_trace";
//         return TCL_ERROR;
//     }
//     BTreeFile *tree = new BTreeFile(status, "btree", attrString, 220, FULL_DELETE, trace);

    if (status != OK) {
        minirel_error->show_errors();
        exit(1);
    }

    // Here are some examples of inserting and deleting records from the tree.

    // insert 20 records in sequence
    int i;
    for (i = 0; i < 20; i++) {
        char rec[220];
        gen_rec(rec, 5*i, 220);

        RID rid;

        rid.pageNo = 5*i;
        rid.slotNo = 5*i+1;
        
        if (tree->insert( (void *)rec, rid) != OK) 
            minirel_error->show_errors(), assert(0);
    }

    int seed = 1996;
    srand(seed);
    int a[20], n;
    for (i = 0; i < 20; i++)
        a[i] = i*5;
    for (i = 0, n = 20; i < 20; i++, n--) {
        int r, t;
        r = rand() % n;
        t = a[i];
        a[i] = a[r];
        a[r] = t;
    }

    // delete all previously inserted record in random order 
    for (i = 0; i < 20; i++) {
        char rec[220];
        RID rid;
        gen_rec(rec, a[i], 220);
        rid.pageNo = a[i];
        rid.slotNo = a[i]+1;
        if (tree->Delete((void*)rec, rid) != OK)
            minirel_error->show_errors();
    }
    
    // insert a record generated from a string key
    char rec[220];
    RID rid;
    gen_rec(rec, "hello", 220);
    tree->insert(rec, rid);
    
    for (i = 0; i < 20; i++)
        a[i] = i*5;
    for (i = 0, n = 20; i < 20; i++, n--) {
        int r, t;
        r = rand() % n;
        t = a[i];
        a[i] = a[r];
        a[r] = t;
    }

    // insert 20 records in random order
    for (i = 0; i < 20; i++) {
        char rec[220];
        gen_rec(rec, a[i], 220);

        RID rid;

        rid.pageNo = a[i];
        rid.slotNo = a[i]+1;
        
        if (tree->insert( (void *)rec, rid) != OK) 
            minirel_error->show_errors(), assert(0);
    }

    delete tree;
    delete minibase_globals;
    
    return 0;
}

