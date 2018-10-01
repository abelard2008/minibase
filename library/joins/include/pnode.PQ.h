// This may look like C code, but it is really -*- C++ -*-
/* 
Copyright (C) 1988 Free Software Foundation
    written by Doug Lea (dl@rocky.oswego.edu)

This file is part of the GNU C++ Library.  This library is free
software; you can redistribute it and/or modify it under the terms of
the GNU Library General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.  This library is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU Library General Public License for more details.
You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef _pnodePQ_h
#ifdef __GNUG__
#pragma interface
#endif
#define _pnodePQ_h 1

//#include <Pix.h>
#include "pnode.defs.h"

typedef void* Pix;

class pnodePQ
{
protected:

  int                   count;

  // Dirty patch by Amit
  int                   fld_no;
  AttrType              fld_type;
  TupleOrder		sort_order;

public:
                        pnodePQ();
  virtual              ~pnodePQ();

  int                   length();                // current number of items
  int                   empty();

  virtual Pix           enq(pnode  item) = 0;      // add item; return Pix
  virtual pnode           deq();                   // return & remove min

  virtual pnode&          front() = 0;             // access min item
  virtual void          del_front() = 0;         // delete min item

  virtual int           contains(pnode  item);     // is item in PQ?

  virtual void          clear();                 // delete all items

  virtual Pix           first() = 0;             // Pix of first item or 0
  virtual void          next(Pix& i) = 0;        // advance to next or 0
  virtual pnode&          operator () (Pix i) = 0; // access item at i
  virtual void          del(Pix i) = 0;          // delete item at i
  virtual int           owns(Pix i);             // is i a valid Pix  ?
  virtual Pix           seek(pnode  item);         // Pix of item

  void                  error(const char* msg);
  virtual int           OK() = 0;                // rep invariant
};


inline pnodePQ::pnodePQ() :count(0) {}

inline pnodePQ::~pnodePQ() {}

inline int pnodePQ::length()
{
  return count;
}

inline int pnodePQ::empty()
{
  return count == 0;
}

#endif
