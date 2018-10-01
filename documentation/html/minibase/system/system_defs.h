// -*- C++ -*-
#ifndef _SYSTEM_DEFS_H
#define _SYSTEM_DEFS_H
/////////////////////////////////////////////////////////////////
//
// filename : system_defs.h    Ranjani Ramamurthy, Dec 4 1995
//
// This defines the class for system startup and all macros
// to define the global variables.
//
//
/////////////////////////////////////////////////////////////////


#ifdef MULTIUSER
class Xaction_manager;
class log;
class shm_log;
class RecoveryMgr;
class LockMgr;
class sh_alloc;
class LockTable;
class ShMemMgr;
#endif
class BufMgr;
class DB;
class Catalog;

#define MINIBASE_MAXARRSIZE 50

class SystemDefs
{

public:
    SystemDefs( Status& status, const char* dbname, unsigned dbpages =0,
                unsigned bufpoolsize =0, const char* replacement_policy =0 );
      /* This constructor uses a default log name and size, for multi-user
         Minibase.  For single-user Minibase, this is the designated
         constructor.  If "dbpages" is 0, the database is opened; if it is
         greater than 0, the database is created with that number of pages. */


    SystemDefs( Status& status, const char* dbname, const char* logname,
                unsigned dbpages, unsigned maxlogsize,
                unsigned bufpoolsize =0, const char* replacement_policy =0 );
      /* This constructor lets you specify all aspects of the system. */


    virtual ~SystemDefs();


    BufMgr*             GlobalBufMgr;
#ifdef MULTIUSER
    ShMemMgr*           GlobalShMemMgr;
    Xaction_manager*    GlobalXactMgr;
    shm_log*            GlobalSHMLog;
#else

      /* We fake shared memory in single-user Minibase to simplify the
         maintenance of the two versions. */
    char* malloc( unsigned size )
        { return new char[size]; }
#define  MINIBASE_SHMEM minibase_globals
#endif

      // These are not in shared memory.
    DB*                 GlobalDB;
#ifdef MULTIUSER
    class log*          GlobalLogMgr;
    RecoveryMgr*        GlobalRecMgr;
    sh_alloc*          	GlobalLockSHMArea;    
    LockMgr*            GlobalLockMgr;    
    LockTable*          GlobalLockTable;   
#endif
    Catalog*            GlobalCatalogPtr;
      /* The global catalog object is declared here, but not allocated by the
         SystemDefs constructor.  If you need to use the catalog, use the
         ExtendedSystemDefs constructor, declared in ext_sys_defs.h. */

    char*               GlobalDBName;
    char*               GlobalLogName;

protected:
    void init( Status& status, const char* dbname, const char* logname,
               unsigned dbpages, unsigned maxlogsize,
               unsigned bufpoolsize, const char* replacement_policy );
};

extern SystemDefs* minibase_globals;

#define  MINIBASE_DB                    (minibase_globals->GlobalDB)
#define  MINIBASE_BM                    (minibase_globals->GlobalBufMgr)

#ifdef MULTIUSER
#define  MINIBASE_SHMEM                 (minibase_globals->GlobalShMemMgr)
#define  MINIBASE_XACTMGR               (minibase_globals->GlobalXactMgr)
#define  MINIBASE_LOGMGR                (minibase_globals->GlobalLogMgr)
#define  MINIBASE_SHMLOG                (minibase_globals->GlobalSHMLog)
#define  MINIBASE_RECMGR                (minibase_globals->GlobalRecMgr)
#define  MINIBASE_LOCKMGR               (minibase_globals->GlobalLockMgr)
#define  MINIBASE_LOCKTABLE             (minibase_globals->GlobalLockTable)
#define  MINIBASE_LOCKSHM               (minibase_globals->GlobalLockSHMArea)
#endif

#define  MINIBASE_DBNAME                (minibase_globals->GlobalDBName)

#endif // _SYSTEM_DEFS_H
