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


#ifndef _pnodeSplayPQ_h
#ifdef __GNUG__
#pragma interface
#endif
#define _pnodeSplayPQ_h 1

#include "pnode.PQ.h"
#include "pnode.SplayNode.h"

class pnodeSplayPQ : public pnodePQ
{
protected:
  pnodeSplayNode*   root;

  pnodeSplayNode*   leftmost();
  pnodeSplayNode*   rightmost();
  pnodeSplayNode*   pred(pnodeSplayNode* t);
  pnodeSplayNode*   succ(pnodeSplayNode* t);
  void            _kill(pnodeSplayNode* t);
  pnodeSplayNode*   _copy(pnodeSplayNode* t);

public:
                  pnodeSplayPQ(int fldNo = 0, AttrType fldType = attrInteger, TupleOrder order = Ascending);
                  pnodeSplayPQ(pnodeSplayPQ& a);
  virtual       ~pnodeSplayPQ();

  Pix           enq(pnode  item);
  pnode           deq(); 

  pnode&          front();
  void          del_front();

  int           contains(pnode  item);

  void          clear(); 

  Pix           first(); 
  Pix           last(); 
  void          next(Pix& i);
  void          prev(Pix& i);
  pnode&          operator () (Pix i);
  void          del(Pix i);
  Pix           seek(pnode  item);

  int           OK();                    // rep invariant
};


inline pnodeSplayPQ::~pnodeSplayPQ()
{
  _kill(root);
}

inline pnodeSplayPQ::pnodeSplayPQ(int fldNo, AttrType fldType, TupleOrder order)
{
  root = 0;
  count = 0;
  fld_no   = fldNo;
  fld_type = fldType;
  sort_order = order;
}

inline pnodeSplayPQ::pnodeSplayPQ(pnodeSplayPQ& b)
{
  count = b.count;
  root = _copy(b.root);
}

inline Pix pnodeSplayPQ::first()
{
  return Pix(leftmost());
}

inline Pix pnodeSplayPQ::last()
{
  return Pix(rightmost());
}

inline void pnodeSplayPQ::next(Pix& i)
{
  if (i != 0) i = Pix(succ((pnodeSplayNode*)i));
}

inline void pnodeSplayPQ::prev(Pix& i)
{
  if (i != 0) i = Pix(pred((pnodeSplayNode*)i));
}

inline pnode& pnodeSplayPQ::operator () (Pix i)
{
  if (i == 0) error("null Pix");
  return ((pnodeSplayNode*)i)->item;
}

inline void pnodeSplayPQ::clear()
{
  _kill(root);
  count = 0;
  root = 0;
}

inline int pnodeSplayPQ::contains(pnode  key)
{
  return seek(key) != 0;
}

#endif
