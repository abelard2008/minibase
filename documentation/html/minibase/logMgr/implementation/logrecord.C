/*
 *  $Id: logrecord.C,v 2.1 1995/05/12 07:19:28 ajitk Exp $
 *
 *  Implementation of class logrecord methods.
 */

#include <ostream>
#include <iomanip.h>
#include <string.h>
#include <ctype.h>

#include "logrecord.h"

/* -------------------------------------------------------------------- */

//
//  Constructors
//

logrecord::logrecord()
{
    //  make buffer all zeros
    memset(buffer, 0, LOG_MAXRECSIZ);

    //
    //  Setup what little we know about the log record header.
    //
    logrechead_ptr              = (logrecheader *) buffer;
    logrechead_ptr->lh_magic    = LOG_MAGIC;
    logrechead_ptr->lh_lsn      = invalid_lsn;
    logrechead_ptr->lh_length   = sizeof(logrecheader);

    //  variable portion of log record is empty.

    data_ptr = buffer + sizeof(logrecheader);
}


logrecord::logrecord(const logrecord &rhs)
{
    logrechead_ptr  = (logrecheader *) buffer;
    *logrechead_ptr = *rhs.logrechead_ptr;
    data_ptr        = buffer + sizeof(logrecheader);
    memcpy(data_ptr, rhs.get_data_ptr(), get_datalength());
}

/* -------------------------------------------------------------------- */

//
//  Assignment operator overload.
//

logrecord &logrecord::operator=(const logrecord &rhs)
{
    if (this == &rhs)
        return *this;

    *logrechead_ptr = *rhs.logrechead_ptr;
    memcpy(data_ptr, rhs.get_data_ptr(), get_datalength());

    return *this;
}

/* -------------------------------------------------------------------- */
    
//
//  Output log record in human readable format.
//

std::ostream &operator<<(std::ostream &out, const logrecord &logrec)
{
    out << "Log Magic: " << hex
        << (unsigned int) logrec.logrechead_ptr->lh_magic << dec;
    out << ", LSN: "     << logrec.logrechead_ptr->lh_lsn << endl;
    out << "Total Log Record Length: " << logrec.logrechead_ptr->lh_length;
    out << ". Data Length: " << logrec.get_datalength();
    
    //
    //  Since we don't know the format of the data, we just dump it
    //  as hex bytes.  Derived classes can overload this method to
    //  output the data part in a more meaningful way.
    //
    for (int i = 0; i < logrec.get_datalength(); ++i) {
        if (i % 8 == 0) {
            //
            //  Dump in ASCII also.
            //
            if (i != 0)
                for (int j = i - 8; j < i; ++j)
                    if (isprint((char) *(logrec.get_data_ptr() + j)))
                        out << (char) *(logrec.get_data_ptr() + j);
                    else
                        out << '.';
            
            out << endl << hex << setw(3) << i << "    ";
        }
        out << setw(2) << (unsigned int) *(logrec.get_data_ptr() + i) << "  ";
    }

    //
    //  Output enough spaces to align with the ASCII dump and then
    //  dump the last line.
    //
    for (int j = i; j % 8 != 0; ++j)
        out << "    ";
    for (j = (i - 1) / 8 * 8; j < i; ++j)
        if (isprint((char) *(logrec.get_data_ptr() + j)))
            out << (char) *(logrec.get_data_ptr() + j);
        else
            out << '.';
    out << dec << endl;
    
    return out; // necessary for statements like: std::cout << lr1 << lr2;
}

