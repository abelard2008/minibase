// This may look like C code, but it is really -*- C++ -*-
/* 
Copyright (C) 1988, 1982 Free Software Foundation
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

#ifndef _pnodeSplayNode
#define _pnodeSplayNode 1
#ifdef __GNUG__
#pragma interface
#endif
#include "pnode.defs.h"

struct pnodeSplayNode
{
  pnode             item;
  pnodeSplayNode*   lt;
  pnodeSplayNode*   rt;
  pnodeSplayNode*   par;
                  pnodeSplayNode(pnode  h, pnodeSplayNode* l=0, pnodeSplayNode* r=0);
                  ~pnodeSplayNode();
};


inline pnodeSplayNode::pnodeSplayNode(pnode  h, pnodeSplayNode* l, pnodeSplayNode* r)
:item(h), lt(l), rt(r), par(0) {}

inline pnodeSplayNode::~pnodeSplayNode() {}

typedef pnodeSplayNode* pnodeSplayNodePtr;

#endif
