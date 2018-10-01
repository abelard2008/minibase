/*
 *  $Id: logshm.h,v 2.1 1995/05/12 07:19:28 ajitk Exp $
 *
 *  Declaration of structures that go into shared memory.
 */

#ifndef _LOGSHM_H
#define _LOGSHM_H

#include <stdlib.h>
#include <stddef.h>

#include "lsn.h"
#include "page.h"
#include "os.h"

/* --------------------------------------------------------------- */

/*
   logfileheader
   
   The logfileheader structure contains the information to be recorded
   in the master log record among other things.  We keep this
   information in the first page of the log file.
*/

struct logfileheader {
    unsigned int    lfh_logfilesiz;     // log file size in pages
    unsigned int    lfh_logtailsiz;     // log tail size in pages
    lsn_t           lfh_last_ckpt_lsn;  // last checkpoint LSN
};

/* -------------------------------------------------------------- */

/*
   shm_log

   Stores in shared memory the logtail and other information to be
   shared by processes that open the same logfile.


   sl_sem
        This binary semaphore is used to maintain mutual exclusion
        during the log open operations.
        
   sl_logtail
        This is the log tail maintained in shared memory.

   sl_firstlsn
        This is the LSN of the first log record that we need to keep
        around.  We keep this with us in shared memory so that we can
        check if we are filling up the log after a wraparound.  We
        get an updated estimate of its value from the recovery manager
        when we need to.

   sl_nextlsn
        The physically last LSN on the log tail is maintained in
        shared memory.  Thus, any transaction can tell what the last
        LSN is.  This is used to compute the LSN to assign the log
        record to be written next. It is also used to inform the buffer
        manager of the LSN of the log record that has been flushed
        last.  Flushing is done a page at a time.  Flushing will be
        done so that the LSN specified by the caller is on disk,
        although more than that may make it to disk.

   sl_logfileheader
        The log file header contains the size of the log file (in
        pages) and the master log record (last checkpoint LSN).

   sl_n_xact
        Number of active transactions that are using the log file.

*/

class shm_log : public SharedRegion {
public:
    Semaphore       sl_sem;
    unsigned char   sl_logtail[PAGESIZE];
    lsn_t           sl_firstlsn, sl_nextlsn;
    logfileheader   sl_logfileheader;
    unsigned int    sl_n_xact;
    
    
    shm_log() : sl_sem()    { lock(); sl_n_xact = 0; unlock(); }
    ~shm_log()              {}
};

/* -------------------------------------------------------------- */

/*
   global_shm_log_ptr

   Pointer to the shared memory segment that will store the
   shm_log structure. Set by the OS infrastructure code during startup.
*/

extern shm_log  *global_shm_log_ptr;

/* -------------------------------------------------------------- */

#endif  // _LOGSHM_H



