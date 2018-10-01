// This may look like C code, but it is really -*- C++ -*-
// WARNING: This file is obsolete.  Use ../SLList.cc, if you can.
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

#ifdef __GNUG__
#pragma implementation
#endif
#include <limits.h>
#include <stream.h>
#include <builtin.h>
#include "String.SLList.h"

void StringSLList::error(const char* msg) const
{
  (*lib_error_handler)("SLList", msg);
}

int StringSLList::length() const
{
  int l = 0;
  StringSLListNode* t = last;
  if (t != 0) do { ++l; t = t->tl; } while (t != last);
  return l;
}

int	StringSLList::contains(String& item) const
{
  for (Pix p=this->first(); p!=0; this->next(p)) {
//	  cout << "***" << this->operator()(p) << " " << item << endl;
	  if ( this->operator()(p)==item )
		return 1;
  }
  return 0;
}

StringSLList::StringSLList(const StringSLList& a)
{
  if (a.last == 0)
    last = 0;
  else
  {
    StringSLListNode* p = a.last->tl;
    StringSLListNode* h = new StringSLListNode(p->hd);
    last = h;
    for (;;)
    {
      if (p == a.last)
      {
        last->tl = h;
        return;
      }
      p = p->tl;
      StringSLListNode* n = new StringSLListNode(p->hd);
      last->tl = n;
      last = n;
    }
  }
}

StringSLList& StringSLList::operator = (const StringSLList& a)
{
  if (last != a.last)
  {
    clear();
    if (a.last != 0)
    {
      StringSLListNode* p = a.last->tl;
      StringSLListNode* h = new StringSLListNode(p->hd);
      last = h;
      for (;;)
      {
        if (p == a.last)
        {
          last->tl = h;
          break;
        }
        p = p->tl;
        StringSLListNode* n = new StringSLListNode(p->hd);
        last->tl = n;
        last = n;
      }
    }
  }
  return *this;
}

void StringSLList::clear()
{
  if (last == 0)
    return;

  StringSLListNode* p = last->tl;
  last->tl = 0;
  last = 0;

  while (p != 0)
  {
    StringSLListNode* nxt = p->tl;
    delete(p);
    p = nxt;
  }
}


Pix StringSLList::prepend(String& item)
{
  StringSLListNode* t = new StringSLListNode(item);
  if (last == 0)
    t->tl = last = t;
  else
  {
    t->tl = last->tl;
    last->tl = t;
  }
  return Pix(t);
}


Pix StringSLList::prepend(StringSLListNode* t)
{
  if (t == 0) return 0;
  if (last == 0)
    t->tl = last = t;
  else
  {
    t->tl = last->tl;
    last->tl = t;
  }
  return Pix(t);
}


Pix StringSLList::append(String& item)
{
  StringSLListNode* t = new StringSLListNode(item);
  if (last == 0)
    t->tl = last = t;
  else
  {
    t->tl = last->tl;
    last->tl = t;
    last = t;
  }
  return Pix(t);
}

Pix StringSLList::append(StringSLListNode* t)
{
  if (t == 0) return 0;
  if (last == 0)
    t->tl = last = t;
  else
  {
    t->tl = last->tl;
    last->tl = t;
    last = t;
  }
  return Pix(t);
}

void StringSLList::join(StringSLList& b)
{
  StringSLListNode* t = b.last;
  b.last = 0;
  if (last == 0)
    last = t;
  else if (t != 0)
  {
    StringSLListNode* f = last->tl;
    last->tl = t->tl;
    t->tl = f;
    last = t;
  }
}

Pix StringSLList::ins_after(Pix p, String& item)
{
  StringSLListNode* u = (StringSLListNode*)p;
  StringSLListNode* t = new StringSLListNode(item);
  if (last == 0)
    t->tl = last = t;
  else if (u == 0) // ins_after 0 means prepend
  {
    t->tl = last->tl;
    last->tl = t;
  }
  else
  {
    t->tl = u->tl;
    u->tl = t;
    if (u == last) 
      last = t;
  }
  return Pix(t);
}


void StringSLList::del_after(Pix p)
{
  StringSLListNode* u = (StringSLListNode*)p;
  if (last == 0 || u == last) error("cannot del_after last");
  if (u == 0) u = last; // del_after 0 means delete first
  StringSLListNode* t = u->tl;
  if (u == t)
    last = 0;
  else
  {
    u->tl = t->tl;
    if (last == t)
      last = u;
  }
  delete t;
}

int StringSLList::owns(Pix p) const
{
  StringSLListNode* t = last;
  if (t != 0 && p != 0)
  {
    do
    {
      if (Pix(t) == p) return 1;
      t = t->tl;
    } while (t != last);
  }
  return 0;
}

String StringSLList::remove_front()
{
  if (last == 0) error("remove_front of empty list");
  StringSLListNode* t = last->tl;
  String res = t->hd;
  if (t == last)
    last = 0;
  else
    last->tl = t->tl;
  delete t;
  return res;
}

int StringSLList::remove_front(String& x)
{
  if (last == 0)
    return 0;
  else
  {
    StringSLListNode* t = last->tl;
    x = t->hd;
    if (t == last)
      last = 0;
    else
      last->tl = t->tl;
    delete t;
    return 1;
  }
}


void StringSLList::del_front()
{
  if (last == 0) error("del_front of empty list");
  StringSLListNode* t = last->tl;
  if (t == last)
    last = 0;
  else
    last->tl = t->tl;
  delete t;
}

int StringSLList::OK() 
{
  int v = 1;
  if (last != 0)
  {
    StringSLListNode* t = last;
    long count = LONG_MAX;      // Lots of chances to find last!
    do
    {
      count--;
      t = t->tl;
    } while (count > 0 && t != last);
    v &= count > 0;
  }
  if (!v) error("invariant failure");
  return v;
}
