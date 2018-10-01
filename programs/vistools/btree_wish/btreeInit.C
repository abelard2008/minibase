#include <tcl.h>
#include <stdlib.h>

#define BT_DEBUG
#include "btfile.h"
#include "buf.h"
#include "db.h"
#include "ext_sys_defs.h"
#include "globals.h"
#include "system_defs.h"

SystemDefs* minibase_globals;
static BTreeFile* tree;
int MINIBASE_RESTART_FLAG=0;

DB* db;    
BufMgr* bm;   
global_errors *minirel_error;

//SpaceMgr* space_mgr;


static void
BTreeInitLib()
{
    minirel_error = new global_errors;
   // space_mgr = new SpaceMgr;

      // Create db and bm.
#if defined(DO_OBSOLETE_CODE)
    Status status;
    db = new DB("btree.minirel",100,status);
    if ( status != OK )
      {
        minirel_error->show_errors();
        exit(1);
      }
    bm = new BufMgr;

    minibase_globals = new ExtendedSystemDefs("NEW","/tmp/qlog",0,20,1,200,"Clock");
#endif /* defined(DO_OBSOLETE_CODE) */
}


static int
BTreeCreate( ClientData , Tcl_Interp* interp, int argc, char** argv )
{
    if (argc != 3)
      {
        interp->result = "wrong # args\nUsage: create_btree <fileName> <keySize>";
        return TCL_ERROR;
      }


      // Create the b-tree.
    Status status;
    tree = new BTreeFile( status, argv[1], attrString, atoi(argv[2]) );
    if (status != OK)
      {
        minirel_error->show_errors();
        exit(1);
      }

    return TCL_OK;
}




static int
BTreePrintPage( ClientData , Tcl_Interp *interp, int argc, char ** argv )
{
    if (argc != 2) {
	interp->result = "wrong # args\nUsage: print_btree_page <page_id>";
	return TCL_ERROR;
    }

    PageId pg = atoi(argv[1]);
    tree->PrintPage(pg);

    return TCL_OK;
}



static int
BTreeInsertRecord( ClientData , Tcl_Interp *interp, int argc, char ** argv )
{
    if ( argc != 4 ) {
	interp->result = "wrong # args\nUsage: insert_btree_record <key> <page> <slot>";
	return TCL_ERROR;
    }

    RID rid = { atoi(argv[2]), atoi(argv[3]) };

    Status status = tree->insert( argv[1], rid );
    if (status != OK) {
        minirel_error->show_errors();
        exit(1);
    }

    return TCL_OK;
}


static int
BTreeDeleteRecord( ClientData , Tcl_Interp *interp, int argc, char ** argv )
{
    if ( argc != 4 ) {
	interp->result = "wrong # args\nUsage: delete_btree_record <key> <page> <slot>";
	return TCL_ERROR;
    }

    RID rid = { atoi(argv[2]), atoi(argv[3]) };

    Status status = tree->Delete( argv[1], rid );
    if (status != OK) {
        minirel_error->show_errors();
        exit(1);
    }

    return TCL_OK;
}

extern "C" {
    int BTree_Init( Tcl_Interp *interp )
        {
	  BTreeInitLib();
          Tcl_CreateCommand( interp, "print_btree_page", BTreePrintPage,
                             NULL, NULL );
          Tcl_CreateCommand( interp, "create_btree", BTreeCreate,
                             NULL, NULL );
          Tcl_CreateCommand( interp, "insert_btree_record", BTreeInsertRecord,
                             NULL, NULL );
          Tcl_CreateCommand( interp, "delete_btree_record", BTreeDeleteRecord,
                             NULL, NULL );
          return TCL_OK;
        }
}
