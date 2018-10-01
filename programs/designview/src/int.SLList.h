// This may look like C code, but it is really -*- C++ -*-
// WARNING: This file is obsolete.  Use ../SLList.h, if you can.
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


#ifndef _intSLList_h
#ifdef __GNUG__
#pragma interface
#endif
#define _intSLList_h 1

#include "Pix.h"

#ifndef _intSLListNode_h
#define _intSLListNode_h 1

struct intSLListNode
{
  int                    hd;
  intSLListNode*         tl;
                         intSLListNode() { }
                         intSLListNode(const int  h, intSLListNode* t = 0);
                         ~intSLListNode() { }
};


inline intSLListNode::intSLListNode(const int  h, intSLListNode* t)
:hd(h), tl(t) {}

typedef intSLListNode* intSLListNodePtr;

#endif


class intSLList
{
protected:
  intSLListNode*        last;

public:
                        intSLList();
                        intSLList(const intSLList& a);
                        ~intSLList();

  intSLList&            operator = (const intSLList& a);

  int                   empty();
  int                   length();

  void                  clear();

  Pix                   prepend(int  item);
  Pix                   append(int  item);

  void                  join(intSLList&);

  Pix                   prepend(intSLListNode*);
  Pix                   append(intSLListNode*);

  int&                  operator () (Pix p);
  Pix                   first();
  void                  next(Pix& p);
  int                   owns(Pix p);
  Pix                   ins_after(Pix p, int  item);
  void                  del_after(Pix p);

  int&                  front();
  int&                  rear();
  int                   remove_front();
  int                   remove_front(int& x);
  void                  del_front();

  void                  error(const char* msg);
  int                   OK();
};

inline intSLList::~intSLList()
{
  clear();
}

inline intSLList::intSLList()
{
  last = 0;
}

inline int intSLList::empty()
{
  return last == 0;
}


inline Pix intSLList::first()
{
  return (last == 0)? 0 : Pix(last->tl);
}

inline void intSLList::next(Pix& p)
{
  p = (p == 0 || p == last)? 0 : Pix(((intSLListNode*)(p))->tl);
}

inline int& intSLList::operator () (Pix p)
{
  if (p == 0) error("null Pix");
  return ((intSLListNode*)(p))->hd;
}

inline int& intSLList::front()
{
  if (last == 0) error("front: empty list");
  return last->tl->hd;
}

inline int& intSLList::rear()
{
  if (last == 0) error("rear: empty list");
  return last->hd;
}

#endif
