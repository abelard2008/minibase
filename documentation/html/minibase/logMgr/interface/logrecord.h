/*
 *  $Id: logrecord.h,v 2.1 1995/05/12 07:19:28 ajitk Exp $
 *
 *  Declaration of the log record class.
 */

#ifndef _LOGRECORD_H
#define _LOGRECORD_H

#include <ostream>

#include "minirel.h"
#include "lsn.h"    

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
    unsigned char   lh_magic;   // the LOG_MAGIC character (see below)
    lsn_t           lh_lsn;     // LSN of log record
    unsigned int    lh_length;  // total length of log record
};

/* ------------------------------------------------------------ */

/*
   logrecord

   The logrecord class defines the structure of a general log record
   and declares functions for setting the log record and retrieving
   information from the log record.

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
        size is declared to store the log record being created. It
        is currently set to 3*PAGESIZE.
        
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
        however, supplies the length of only the data portion.) The size
        of the user's data portion cannot be more that LOG_MAXRECSIZ minus
        the size of the logrecheader.

   buffer
        The buffer in which the log record is stored.

   set_lsn
        This is used only by the log class.  It will use this to set
        the LSN field of the log record.  Storing the LSN in the log
        record is not strictly necessary since LSN is implicit by virtue 
        of the position of the log record.  However, it will help during
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

    const unsigned int LOG_MAXRECSIZ = PAGESIZE * 3;
    
    void set_datalength(unsigned int logrecdatalen) {
        logrechead_ptr->lh_length = logrecdatalen + sizeof(logrecheader);
    }

    lsn_t          get_lsn() const        { return logrechead_ptr->lh_lsn; }
    unsigned int   get_datalength() const { return logrechead_ptr->lh_length - 
                                                   sizeof(logrecheader); }
    unsigned char *get_data_ptr() const   { return data_ptr; }

    logrecord     &operator=(const logrecord &rhs);

    friend std::ostream &operator<<(std::ostream &out, const logrecord &logrec);
    
    friend class log;
    
protected:
    const unsigned char     LOG_MAGIC = 0xFF;
    const unsigned char     LOG_END   = 0x0;

    logrecheader  *logrechead_ptr;
    unsigned char *data_ptr;
    unsigned char  buffer[LOG_MAXRECSIZ];

    void           set_lsn(const lsn_t &lsn) { logrechead_ptr->lh_lsn = lsn; }
};

/* ---------------------------------------------------------------- */

#endif  //  _LOGRECORD_H
