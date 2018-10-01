/* We put the extended system defs in a separate file from the normal system
   defs to remove the dependency on catalogs for users of the plain system
   defs. */
#include "minirel.h"
#include "ext_sys_defs.h"
#include "db.h"
#include "maincatalog.h"
#include "ii_systemcatalogs.h"

ExtendedSystemDefs::ExtendedSystemDefs( Status& status, const char* dbname, unsigned dbpages,
                                        unsigned bufpoolsize,
                                        const char* replacement_policy )
    : SystemDefs(status,dbname,dbpages,bufpoolsize,replacement_policy)
{
    init( status, dbpages > 0 );
}

ExtendedSystemDefs::ExtendedSystemDefs( Status& status, const char* dbname,
                                        const char* logname, unsigned dbpages,
                                        unsigned maxlogsize, unsigned bufpoolsize,
                                        const char* replacement_policy )
 : SystemDefs(status,dbname,logname,dbpages,maxlogsize,bufpoolsize,replacement_policy)
{
    init( status, dbpages > 0 );
}


ExtendedSystemDefs::~ExtendedSystemDefs()
{
    delete GlobalCatalogPtr;
    GlobalCatalogPtr = 0;
    delete systemCatalogs;
    systemCatalogs = 0;
}


void ExtendedSystemDefs::init( Status& status, bool initCatalog )
{

      // Create the global Catalog Pointer..
    if ( status == OK )
        GlobalCatalogPtr = new Catalog(status);
    if ( status == OK && initCatalog )
        status = GlobalCatalogPtr->initialize();
    if ( status == OK )
      {
        delete systemCatalogs;
        systemCatalogs = new ii_MinibaseSystemCatalogs;
      }

    
}
