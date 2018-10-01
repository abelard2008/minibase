/*
 *  $Id: logerror.C,v 2.1 1995/05/12 07:19:28 ajitk Exp $
 *
 *  Log error messages
 */

#include "logerror.h"

/*
   log_mgr_msgs

   Messages that the log manager returns on errors.
*/

const char *log_mgr_msgs[] = {
    "Bad log filename",
    "Cannot create log file",
    "End of log file reached",
    "Log tail flush failed",
    "Invalid LSN",
    "Log read failed",
    "Log record too long",
    "Log header page bad/missing",
    "Insufficient memory",
    "Insufficient shared memory",
    "Log file full",
    "Log open failed",
    "LSN name space exhausted",
    "Log write failed",
    "Recovery failed"
};











