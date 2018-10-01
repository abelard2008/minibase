/*
 *  $Id: logrecord.h,v 1.1 1996/02/11 04:25:04 luke Exp $
 *
 *  $Log: logrecord.h,v $
 *  Revision 1.1  1996/02/11 04:25:04  luke
 *  Initial check-in of minibase
 *
 * Revision 1.7  1995/05/10  13:30:32  cjin
 * modified output fn
 *
 * Revision 1.6  1995/05/09  08:45:47  cjin
 * incorporated new error protocol\
 *
 * Revision 1.5  1995/05/07  21:52:02  ajitk
 * Minor stylistic changes.
 *
 * Revision 1.4  1995/05/06  17:23:51  cjin
 * changed MAXLOGSIZE to PAGE_SIZE*3
 *
 * Revision 1.3  1995/05/01  06:02:46  cjin
 * removed #include "logshm.h" and #include "db.h"
 *
 * Revision 1.2  1995/05/01  05:35:51  ajitk
 * Class logrecord is now in a separate file.
 *
 *
 *  Declaration of the log record class.
 */

#ifndef _LOGRECORD_H
#define _LOGRECORD_H

#include <ostream>

#include "minirel.h"    //  global data types
#include "lsn.h"        //  definition of LSN

/* ------------------------------------------------------------ */

/*
 *  class log -- forward declaration.
 */

class log;

/* ------------------------------------------------------------ */

/*
   logrecheader

   We maintain the log record in a large buffer in a flattened form.
   This structure defines the fixed header portion of the log record
   for easy access.
*/

struct logrecheader {
    unsigned char   lh_magic;
    lsn_t           lh_lsn;
    unsigned int    lh_length;
};

/* ------------------------------------------------------------ */

/*
   logrecord

   The logrecord class defines the structure of a general log record
   and declares functions for reading and writing the record.

   LOG_MAGIC, LOG_END
        The LOG_MAGIC value is used at the beginning of
        every log record.  That way, during recovery, we can decide
        whether we have reached the end of the log.  LOG_END is a
        value written at the end of a log record.  It is overwritten
        by the LOG_MAGIC of the next log record appended to the log.
        Thus, LOG_END is not part of the log record.  This also
        facilitates detecting end of log.

   LOG_MAXRECSIZ
        Maximum size of a log record in bytes.  A buffer of this
        size is declared to store the log record being created.
        
   logrechead_ptr
        A large buffer will be used to store the log record in a
        flattened form.  This pointer points to the start of the log
        header portion of the buffer.

   data_ptr
        The log knows nothing about the contents of the log record.
        All it knows is that there is a fixed portion (accessed through
        the logrechead_ptr) and a variable size portion.  The variable
        size is defined differently for normal updates, commit
        records, etc.  The recovery manager will suitably derive from
        this class and define various kinds of log records.  The
        data portion is a byte array representing the variable part of
        the log record.  (The length field stored in the header is the
        length of the entire log record.  The user of this class,
        however, supplies the length of only the data portion.)

   buffer
        The buffer in which the log record is stored.

   set_lsn
        This is used only by the log class.  It will use this to set
        the LSN field of the log record.  Storing the LSN in the log
        record is not strictly necessary.  But it will help while
        outputting the log record in human readable format.
        
   set_datalength
        This sets the length of the data portion of the log record.
        What is actually stored in the log record is the length of the
        entire log record.
        
   Constructors
        The first constructor initializes the data members to indicate
        an invalid log record.  Fields can later be filled in using the
        set_* methods or directly by derived classes.
        The second constructor allows initialization of one log record
        from another.

   get_*
        Used to retrieve the appropriate fields of the log record.

   operator=
        Assigns a log record to another.
        
   operator<<
        Outputs the log record in a human readable format.

   class log is a friend because it has to be able to set the LSN.
*/

class logrecord {
public:
    logrecord();
    logrecord(const logrecord &rhs);
    ~logrecord() {}

    const unsigned int  LOG_MAXRECSIZ   = MINIBASE_PAGESIZE * 3;
    
    void set_datalength(unsigned int logrecdatalen) {
        logrechead_ptr->lh_length = logrecdatalen + sizeof(logrecheader);
    }

    lsn_t        get_lsn() const        { return logrechead_ptr->lh_lsn; }
    unsigned int get_datalength() const { return logrechead_ptr->lh_length - 
                                                 sizeof(logrecheader); }
    unsigned char *get_data_ptr() const { return data_ptr; }

    logrecord   &operator=(const logrecord &rhs);

    friend std::ostream &operator<<(std::ostream &out, const logrecord &logrec);
    friend class log;
    
protected:
    const unsigned char  LOG_MAGIC = 0xFF;
    const unsigned char  LOG_END   = 0x0;

    logrecheader  *logrechead_ptr;
    unsigned char *data_ptr;
    unsigned char  buffer[LOG_MAXRECSIZ];

    void          set_lsn(const lsn_t &lsn) { logrechead_ptr->lh_lsn = lsn; }
};

/* ---------------------------------------------------------------- */

#endif  //  _LOGRECORD_H
