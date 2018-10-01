/*
 *  $Id: lsn.h,v 1.3 1996/05/02 22:36:04 whuang Exp $
 *
 *  $Log: lsn.h,v $
 *  Revision 1.3  1996/05/02 22:36:04  whuang
 *  added full delete algorithm for btree.
 *  integrated with modified visualization tool.
 *
 *  Revision 1.2  1996/03/06 20:31:14  ramamurt
 *  log changes
 *
 *  Revision 1.1  1996/02/11 04:25:06  luke
 *  Initial check-in of minibase
 *
 * Revision 1.13  1995/05/10  13:32:05  cjin
 * ajit checked out. I checked in.
 *
 * Revision 1.12  1995/05/09  04:25:31  ajitk
 * No change.
 *
 * Revision 1.11  1995/05/07  21:49:59  ajitk
 * invalid() method removed.  Declared a const invalid_lsn which
 * is used everywhere an invalid_lsn is required.
 *
 * Revision 1.10  1995/05/06  21:29:44  cjin
 * included set_lsn() to set lsn values. For testing only.
 *
 * Revision 1.9  1995/05/06  16:18:08  cjin
 * added invalid() method to return an invalid lsn
 *
 * Revision 1.8  1995/04/27  02:13:22  ajitk
 * Removed definition of True and False.  We use true and false
 * instead.
 * Also made is_invalid a const member function.
 *
 * Revision 1.7  1995/04/26  19:20:26  cjin
 * added comments for method
 *
 * Revision 1.6  1995/04/26  19:11:31  cjin
 * it compiles ok now
 *
 * Revision 1.5  1995/04/26  16:43:49  cjin
 * added a few set/get private methods
 *
 * Revision 1.4  1995/04/26  12:02:35  cjin
 * included "minirel.h"
 *
 * Revision 1.3  1995/04/12  07:32:29  cjin
 * Modified by cjin on April 12 - minimal modification
 *
 * Revision 1.2  1995/04/07  06:38:04  ajitk
 * Since the log file is now a page file, the LSN maintains the page id
 * and offset within the page rather than a file offset.
 * Also added a method to determine invalid LSNs
 * Finally, normal users can access only the default constructor.
 * class log accesses the constructor which initializes the data members.
 * This prevents normal users from fiddling with LSNs.
 *
 * Revision 1.1  1995/04/01  06:57:26  ajitk
 * Initial revision
 *
 *  Filename: lsn.h
 *
 *  Definition of LSN.  LSN is a wrap count (how many times around
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
class RecoveryMgr;

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
        The other constructor is private because the only class that
        knows about the contents of the LSN is log.

   Set/Get methods
        Use internally to set and get the different parts of an LSN.   
      
   Boolean operators
        Comparison of two LSNs for equality, etc., is not
        straightforward since a wrap count is involved.  The operators
        are overloaded to help users compare LSNs.  Each operator
        compares its LHS and RHS.  Returns true if the comparison is
        successful, false otherwise.

   operator=
        Assignment operator for LSN.  The lhs is made equal to the rhs.

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
        Outputs the value of the LSN to the output stream.
        Returns the std::ostream passed in if there is no error, returns 0
        otherwise.
        
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
    PageId   get_pageid() const     { return page_id; }
    unsigned int get_offset() const { return offset; }
    
    void set_wrap(unsigned int wrap)  { wrap_count = wrap; }
    void set_pageid(PageId page)      { page_id = page; }
    void set_offset(unsigned int off) { offset = off; }

public:
    lsn_t(unsigned int wrap, PageId page, unsigned int off);
    lsn_t();

    // for testing purposes only
    void set_lsn(unsigned int wrap, PageId page, unsigned int off) {
        set_wrap(wrap);
        set_pageid(page);
        set_offset(off);
    }
    PageId   get_page_id() const     { return page_id; }
    
    bool    operator==(const lsn_t &rhs) const;
    bool    operator!=(const lsn_t &rhs) const;
    bool    operator<(const lsn_t &rhs) const;
    bool    operator>(const lsn_t &rhs) const;
    bool    operator<=(const lsn_t &rhs) const;
    bool    operator>=(const lsn_t &rhs) const;

    lsn_t   &operator=(const lsn_t &rhs);

    bool    is_invalid() const;
    
    // to display lsn in a human readable form
    friend std::ostream &operator<<(std::ostream &out, const lsn_t &lsn);
    friend class log;
    friend class RecoveryMgr;
    void print() { 
        //	std::cout << (*this); 
    }
};

const lsn_t invalid_lsn = lsn_t();

/* ------------------------------------------------------------ */

#endif  //  _LSN_H
