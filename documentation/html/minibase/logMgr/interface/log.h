/*
 *  $Id: log.h,v 2.1 1995/05/12 07:19:28 ajitk Exp $
 *
 *  Declaration of class log.
 */

#ifndef _LOG_H
#define _LOG_H

#include <ostream>
#include <stdlib.h>

#include "lsn.h"      
#include "logrecord.h"
#include "logshm.h"   
#include "rawfile.h"  
#include "logerror.h" 

/* ------------------------------------------------------------- */
    
/*
   log

   This class access the log tail in shared memory, allows reading and
   writing of log records and flushing the log tail.

   logfile_ptr
        Pointer to the log file.  Each transaction will have its own
        Rawfile object but they will all be accessing the same rawfile.
        
   open_called
        If this is true, the constructor called was log::log(dbname).
        Otherwise, it is the create constructor.  The difference is
        that the destructor has to behave differently.  This is
        because the create constructor is special and doesn't
        initialize the log tail and other shared memory data
        structures.
        
   next_lsn_less_1
        Compute the LSN immediately preceding the LSN to be assigned
        next and returns that.

   pages_between
        Calculates the number of pages between 2 LSNs.

   find_log_size
        Find the size of the log used so far (in pages).   
        
   internal_flush
        This method does the actual flushing.  flush and bufmgr_flush
        take care of mutual exclusion, informing the buffer manager
        if necessary, etc.

   Constructors
        First constructor will open an existing log file and setup the
        log tail.
        The second constructor will create a new log file.  This
        constructor is called only by an offline create utility which
        doesn't run transactions.  Therefore, this constructor doesn't
        bother about creating log tails, etc.

   Destructor
        The destructor closes the log file if it was created using the
        second constructor.
        Otherwise, the destructor will flush the log tail if this is
        the last transaction to use the log file; it will
        close the log file and destroy the log tail if this is the
        last transaction to use the corresponding DB.

   read
        Reads and returns a log record given the LSN. Returns OK on
        success, LOGMGR on failure.
        
   write
        Appends log record to log tail.  Log tail may get flushed if
        full.  Returns OK on success, LOGMGR otherwise.  If write
        succeeds, assigned_lsn contains the LSN assigned to the log
        record.  Otherwise, assigned_lsn is set to invalid_lsn.
        The is_rollback field is used to determine if a rollback is
        in progress or not.  If set to true, a rollback is in progress,
        and log writes can proceed until the log is physically full.
        If false, log writes can proceed only until the log is half full.
        That way, rollbacks can always complete successfully.

   read_next_logrec
        Given the LSN of the current record, read the next log record
        and fill the logrecord passed in.  Return OK on success,
        LOGMGR on failure.  LOGMGR includes end of log indication.  This
        method sets nextLSN to the LSN of the end of the log if the
        end is reached.

   read_next_logrec
        Given the current log record, find the LSN of the next log
        record and read that record in.  Return OK on success and
        LOGMGR if the end of log has been reached.  This method sets
        nextLSN to the LSN of the end of the log if the end is reached.

   get_next_lsn
        On successful return, will assign lsn to the next LSN that
        will be assigned, and return OK.  Otherwise sets lsn to
        invalid_lsn and returns LOGMGR.

   set_newckpt_lsn, get_lastckpt_lsn
        Master log record manipulation.  Set gives write access, get
        gives read access.  Set also flushes the master log record to
        disk.  Returns OK on success, LOGMGR otherwise.
    
   flush
        This method is called by everyone except the buffer manager.
        It will flush the entire log tail and inform the buffer
        manager of the LSN of the last log record that made it to
        disk.  Method returns OK on success, LOGMGR on failure.
   
   bufmgr_flush
        The buffer manager calls this method when it wants to flush
        the log tail to disk.  The method will flush the entire log
        tail to disk and return the LSN of the last log record that
        made it in the flushed_lsn parameter.  flushed_lsn is set to
        an invalid LSN on error.  Returns OK on success, LOGMGR on
        error.

*/

class log {
private:
    Rawfile *logfile_ptr;
    bool     open_called;
    
    lsn_t    next_lsn_less_1();
    int      pages_between(lsn_t startlsn, lsn_t stoplsn, int logfilesize,
                           PageId headerpage);
    int      find_log_size(lsn_t startlsn, lsn_t stoplsn, int logfilesize,
                           PageId headerpage);
    lsn_t    internal_flush();
    
public:
    log(const char *dbname, Status &error_status);
    log(const char *dbname, unsigned int maxlogsize,
        unsigned int logtailsize, Status &error_status);
    ~log();

    Status read(const lsn_t &lsn, logrecord &logrec);
    Status write(logrecord &logrec, bool is_rollback, lsn_t &assigned_lsn);
    Status read_next_logrec(const lsn_t &currentlsn, logrecord &logrec);
    Status read_next_logrec(const logrecord &curr_logrec,
                            logrecord &next_logrec);
    
    Status get_next_lsn(lsn_t &nextlsn);

    Status get_lastckpt_lsn(lsn_t &lastckpt_lsn);
    Status set_newckpt_lsn(const lsn_t &newckpt);

    Status flush();
    Status bufmgr_flush(lsn_t &flushed_lsn);
};

/* ------------------------------------------------------------- */

#endif  //  _LOG_H
