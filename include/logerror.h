/*
 *  $Id: logerror.h,v 1.2 1996/03/31 18:27:18 luke Exp $
 *
 *  $Log: logerror.h,v $
 *  Revision 1.2  1996/03/31 18:27:18  luke
 *  Added MULTIUSER tests and singleuser build code
 *
 *  Revision 1.1  1996/02/11 04:25:02  luke
 *  Initial check-in of minibase
 *
 * Revision 1.2  1995/05/10  13:29:48  cjin
 * ajitk made changes.
 *
 * Revision 1.1  1995/05/09  08:48:14  cjin
 * Initial revision
 *
 *
 *  Declaration of Log Manager errors
 */

#ifndef _LOGERROR_H
#define _LOGERROR_H


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

#endif










