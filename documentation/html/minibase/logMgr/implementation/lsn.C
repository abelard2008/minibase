/*
 *  $Id: lsn.C,v 2.1 1995/05/12 07:19:28 ajitk Exp $
 *
 *  Code for manipulating log sequence number (LSN).  Implements 
 *  methods of class lsn_t.
 */

#include "lsn.h"

/* --------------------------------------------------------------- */

//
//  constructors
//

lsn_t::lsn_t(unsigned int wrap, PageId page, unsigned int off)
{
    //
    //  check bounds
    //
    if (wrap == LSN_INVALID_WRAP  ||  page == LSN_INVALID_PAGEID  ||
        off == LSN_INVALID_OFFSET)
        *this = invalid_lsn;
    else {
        set_wrap(wrap);
        set_pageid(page);
        set_offset(off);
    }
}

lsn_t::lsn_t()
{
    set_wrap(LSN_INVALID_WRAP);
    set_pageid(LSN_INVALID_PAGEID);
    set_offset(LSN_INVALID_OFFSET);
}

/* ---------------------------------------------------------------- */

//
//  comparison operations on LSNs
//

bool lsn_t::operator==(const lsn_t &rhs) const
{
    return get_wrap()   == rhs.get_wrap()    &&
           get_pageid() == rhs.get_pageid()  &&
           get_offset() == rhs.get_offset();
}

bool lsn_t::operator!=(const lsn_t &rhs) const
{
    return get_wrap()   != rhs.get_wrap()    ||
           get_pageid() != rhs.get_pageid()  ||
           get_offset() != rhs.get_offset();
}

bool lsn_t::operator<(const lsn_t &rhs) const
{
    if (get_wrap() < rhs.get_wrap())
        return true;
    else if (get_wrap() == rhs.get_wrap()  &&  get_pageid() < rhs.get_pageid())
        return true;
    else if (get_wrap()   == rhs.get_wrap()    &&
             get_pageid() == rhs.get_pageid()  &&
             get_offset() <  rhs.get_offset())
        return true;
    else
        return false;
}

bool lsn_t::operator>(const lsn_t &rhs) const
{
    if (get_wrap() > rhs.get_wrap())
        return true;
    else if (get_wrap() == rhs.get_wrap()  &&  get_pageid() > rhs.get_pageid())
        return true;
    else if (get_wrap()   == rhs.get_wrap()    &&
             get_pageid() == rhs.get_pageid()  &&
             get_offset() >  rhs.get_offset())
        return true;
    else
        return false;
}

bool lsn_t::operator<=(const lsn_t &rhs) const
{
    if (get_wrap() < rhs.get_wrap())
        return true;
    else if (get_wrap()   == rhs.get_wrap()  &&
             get_pageid() <  rhs.get_pageid())
        return true;
    else if (get_wrap()   == rhs.get_wrap()    &&
             get_pageid() == rhs.get_pageid()  &&
             get_offset() <= rhs.get_offset())
        return true;
    else
        return false;
}

bool lsn_t::operator>=(const lsn_t &rhs) const
{
    if (get_wrap() > rhs.get_wrap())
        return true;
    else if (get_wrap() == rhs.get_wrap()  &&  get_pageid() > rhs.get_pageid())
        return true;
    else if (get_wrap()   == rhs.get_wrap()    &&
             get_pageid() == rhs.get_pageid()  &&
             get_offset() >= rhs.get_offset())
        return true;
    else
        return false;
}

/* ----------------------------------------------------------------- */

//
//  assignment operation on lsn
//

lsn_t &lsn_t::operator=(const lsn_t &rhs)
{
    if (this == &rhs)
        return *this;
    
    set_wrap(rhs.get_wrap());
    set_pageid(rhs.get_pageid());
    set_offset(rhs.get_offset());

    return *this;  // necessary for expressions like: lsn1 = lsn2 = lsn3;
}

/* ----------------------------------------------------------------- */

//
//  check if lsn is invalid
//

bool lsn_t::is_invalid() const
{
    return get_wrap()   == LSN_INVALID_WRAP     ||
           get_pageid() == LSN_INVALID_PAGEID   ||
           get_offset() == LSN_INVALID_OFFSET;
}

/* ------------------------------------------------------------------ */

//
//  display lsn in a human readable format
//
std::ostream &operator<<(std::ostream &out, const lsn_t &lsn)
{
    if (lsn.is_invalid())
        out << "<invalid LSN>";
    else
        out << "Wrap Count = " << lsn.get_wrap() 
            << ", PageId = "   << lsn.get_pageid() 
            << ", Offset = "   << lsn.get_offset();

    return out;  // necessary for statements like std::cout << lsn1 << lsn2;
}

