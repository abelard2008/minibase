/*
 *  $Id: lsn.h,v 2.1 1995/05/12 07:17:39 ajitk Exp $
 *
 *  Declaration of class LSN. LSN is a wrap count (how many times around
 *  the bounded log have we gone), the page number within the log file,
 *  and the offset within the page.  We use the abstraction of the log
 *  being maintained on a page file so that we can move the log to a
 *  dedicated disk later on easily.
 */

#ifndef _LSN_H
#define _LSN_H

#include <limits.h>
#include <ostream>

#include "minirel.h"

/* ------------------------------------------------------------ */

/*
 *  class log -- forward declaration.
 */

class log;

/* ------------------------------------------------------------ */

/*
   lsn_t

   This describes the LSN.  The LSN is internally a wrap count, a page
   number and an offset within the page.  This allows wraparound in a
   bounded log.

   Constructors
        Default constructor initializes the LSN to an invalid value.
        The second constructor allows setting of all values.
        The default constructor is public so that users can
        declare an object of class lsn_t as:
                lsn_t lsn;
        The other constructor allows setting of the wrap count,
        the page ID, and the offset within the page. It is also made
        public to allow the creation of a specific (but proper) LSN by
        anyone.

   Set/Get methods
        Used internally to set and get the different components of an LSN.   
      
   Boolean operators
        Comparison of two LSNs for equality, etc., is not
        straightforward since a wrap count is involved.  The operators
        are overloaded to help users compare LSNs.  Each operator
        compares its LHS and RHS.  Returns true if the comparison is
        successful, false otherwise.

   operator=
        Assignment operator for LSN.  The LSN is assigned the value of
        the RHS.

   is_invalid
        Returns true if the LSN is invalid.  false otherwise.  An
        invalid LSN is one whose default constructor has been called
        (but nothing else has been done to set the values).  Invalid
        LSNs are used by the log class flush methods to indicate
        failure of flush operations.  Invalid LSNs have
        LSN_INVALID_WRAP, LSN_INVALID_PAGEID, LSN_INVALID_OFFSET as
        their data contents.  No valid LSN can have these.  We
        guarantee this by ensuring that wrap count never exceeds
        UINT_MAX - 1.
        
   operator<<
        Outputs the value of the LSN in a nice form to the output
        stream.
        
   The log class is declared as a friend because it directly
   manipulates LSNs.
*/

class lsn_t {
private:
    const unsigned int  LSN_INVALID_WRAP    = UINT_MAX;
    const PageId        LSN_INVALID_PAGEID  = INT_MAX;
    const unsigned int  LSN_INVALID_OFFSET  = UINT_MAX;
    
    unsigned int    wrap_count;
    PageId          page_id;
    unsigned int    offset;
    
    unsigned int get_wrap() const   { return wrap_count; }
    PageId   get_pageid() const     { return page_id;    }
    unsigned int get_offset() const { return offset;     }
    
    void set_wrap(unsigned int wrap)    { wrap_count = wrap; }
    void set_pageid(PageId page)        { page_id = page;    }
    void set_offset(unsigned int off)   { offset = off;      }

public:
    lsn_t();
    lsn_t(unsigned int wrap, PageId page, unsigned int off);
    ~lsn_t() {}
    
    bool operator==(const lsn_t &rhs) const;
    bool operator!=(const lsn_t &rhs) const;
    bool operator< (const lsn_t &rhs) const;
    bool operator> (const lsn_t &rhs) const;
    bool operator<=(const lsn_t &rhs) const;
    bool operator>=(const lsn_t &rhs) const;

    lsn_t &operator=(const lsn_t &rhs);

    bool   is_invalid() const;
    
    friend std::ostream &operator<<(std::ostream &out, const lsn_t &lsn);
    
    friend class log;
};

const lsn_t invalid_lsn = lsn_t();  // a useful and handy invalid LSN

/* ------------------------------------------------------------ */

#endif  //  _LSN_H
