/*
 *  $Id: logerror.h,v 2.1 1995/05/12 07:19:28 ajitk Exp $
 *
 *  Declaration of Log Manager errors
 */

#ifndef _LOGERROR_H
#define _LOGERROR_H

/*
   log_mgr_errors

   Index into the array of messages that the log manager returns when
   various errors occur.
   Remember to update log_mgr_msgs when this enum is updated.
*/

enum log_mgr_errors {
    E_LOG_BADDBNAME,
    E_LOG_CREATELOGFILE,
    E_LOG_EOF,
    E_LOG_FLUSHFAILED,
    E_LOG_INVALIDLSN,
    E_LOG_LOGFILEREAD,
    E_LOG_LOGRECTOOLONG,
    E_LOG_NOHEADERPAGE,
    E_LOG_NOMEM,
    E_LOG_NOSHM,
    E_LOG_NOSPC,
    E_LOG_OPENLOGFILE,
    E_LOG_NOMORELSN,
    E_LOG_LOGFILEWRITE,
    E_LOG_RECOVERYFAILED
};

extern const char *log_mgr_msgs[];

#endif  // _LOGERROR_H











