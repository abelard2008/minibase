/*
 *  $Id: logshm.h,v 1.1 1996/02/11 04:25:05 luke Exp $
 *
 *  $Log: logshm.h,v $
 *  Revision 1.1  1996/02/11 04:25:05  luke
 *  Initial check-in of minibase
 *
 * Revision 1.11  1995/05/10  13:31:27  cjin
 * ajit made some changes?
 *
 * Revision 1.10  1995/05/09  04:23:46  ajitk
 * Added the sl_sem semaphore to mutually exclude opens.
 *
 * Revision 1.9  1995/05/07  21:49:14  ajitk
 * Minor corrections.
 *
 * Revision 1.8  1995/05/06  17:14:01  cjin
 * removed perDBloginfo class
 * renamed shm_log_header to just shm_log
 * added to shm_log class: sl_logtail, sl_firstlsn, sl_nextlsn,
 *                         sl_logfileheader, sl_num_xact
 * deleted logshm.C: no need for it anymore
 * renamed global pointer as global_shm_log_ptr
 * constructor of shm_log class now just sets sl_num_xact = 0
 *
 * Revision 1.7  1995/05/03  07:14:08  cjin
 * included empty destructor for shm_log_header
 *
 * Revision 1.6  1995/05/02  00:25:09  cjin
 * included "db.h"
 * uncommented head() and free_head()
 *
 * Revision 1.5  1995/05/01  21:56:24  cjin
 * commented out #include "db.h"
 * added to perDB_loginfo class: char name[MAXNAME];
 *                               pdli_prev, pdli_next
 *                               void print(); <- for testing purposes
 * modified shm_log_header class: made list and freelist public
 *    commented out head() and free_head() methods
 *    included insert(), remove(), display() for testing purposes
 *    (may be expanded into real member functions)
 *
 * Revision 1.4  1995/05/01  05:38:05  ajitk
 * Minor changes.
 *
 * Revision 1.3  1995/04/29  19:14:14  ajitk
 * 1. Changed perDB_loginfo to a class and added operators new and
 *    delete.
 * 2. Changed shm_log_header::shm_log_header to lock before initializing
 *    list and freelist and unlock after.
 *
 * Revision 1.2  1995/04/29  18:55:52  ajitk
 * shm_log_header::head returns a reference to shm_log_header::list
 * so that list can be changed.
 * Also, added method shm_log_header::free_head to return a
 * reference to shm_log_header::freelist.
 *
 * Revision 1.1  1995/04/29  18:50:02  ajitk
 * Initial revision
 *
 *
 *  Declaration of structures that go into shared memory.
 */

#ifndef _LOGSHM_H
#define _LOGSHM_H

#include <stdlib.h>
#include <stddef.h>

#include "minirel.h"
#include "os.h"
#include "page.h"
#include "lsn.h"
#include "xaction_mgr.h"


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
    lsn_t           lfh_last_ckpt_lsn;  // last checkpoint lsn
};

/* -------------------------------------------------------------- */

/*
   shm_log

   Stores in shared memory the logtail and other information to be
   shared by processes that open the same db.


   sl_logtail
        This is the tail maintained in shared memory.

   sl_firstlsn
        This is the LSN of the first log record that we need to keep
        around.  We keep this with us in shared memory so that we can
        check if we are filling up the log after a wraparound.  We
        get the information from the recovery manager when we need to.

   sl_nextlsn
        The physically last LSN on the log tail is maintained in
        shared memory.  Thus, any transaction can tell what the last
        LSN is.  This is used to compute the LSN to assign the log
        record to be written.  It is also used to inform the buffer
        manager of the LSN of the log record that has been flushed
        last.  Flushing is done a page at a time.  Flushing will be
        done so that the LSN specified by the caller is on disk.  But
        more than that may make it to disk.

   sl_logfileheader
        The log file header contains the size of the log file (in
        pages) and the master log record.

   sl_n_xact
        Number of active transactions that are using the log DB.

*/

//class SharedRegion;
class shm_log : public SharedRegion {
public:
    Semaphore       sl_sem;
    unsigned char   sl_logtail[MINIBASE_PAGESIZE];
    lsn_t           sl_firstlsn, sl_nextlsn;
    logfileheader   sl_logfileheader;
    unsigned int    sl_n_xact;
    
    
    shm_log() : sl_sem()    { lock(); sl_n_xact = 0; unlock(); }
    ~shm_log()              {}

    // for testing purposes
    void display() {
        std::cout << "Number of xacts is " << sl_n_xact << endl;
    }
};

/* -------------------------------------------------------------- */

/*
   global_shm_log_ptr

   Pointer to the shared memory segment that will store the
   shm_log_header.  Set by the OS infrastructure code during startup.
*/

extern shm_log  *global_shm_log_ptr;

/* -------------------------------------------------------------- */

#endif  // _LOGSHM_H



