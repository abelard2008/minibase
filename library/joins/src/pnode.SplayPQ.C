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

#ifdef __GNUG__
#pragma implementation
#endif
//#include <stream>
#include "pnode.SplayPQ.h"


/* 

 struct to simulate the special `null' node in the Sleater & Tarjan JACM 1985
 splay tree algorithms

 All routines use a version of their `simple top-down' splay alg. (p 669)

*/

struct _dummySplayNode
{
  pnodeSplayNode*    lt;
  pnodeSplayNode*    rt;
  pnodeSplayNode*    par;
} _dummy_null;


/*
 traversal primitives
*/


pnodeSplayNode* pnodeSplayPQ::leftmost()
{
  pnodeSplayNode* t = root;
  if (t != 0) while (t->lt != 0) t = t->lt;
  return t;
}

pnodeSplayNode* pnodeSplayPQ::rightmost()
{
  pnodeSplayNode* t = root;
  if (t != 0) while (t->rt != 0) t = t->rt;
  return t;
}

pnodeSplayNode* pnodeSplayPQ::succ(pnodeSplayNode* t)
{
  if (t == 0)
    return 0;
  if (t->rt != 0)
  {
    t = t->rt;
    while (t->lt != 0) t = t->lt;
    return t;
  }
  else
  {
    for (;;)
    {
      if (t->par == 0 || t == t->par->lt)
        return t->par;
      else
        t = t->par;
    }
  }
}

pnodeSplayNode* pnodeSplayPQ::pred(pnodeSplayNode* t)
{
  if (t == 0)
    return 0;
  else if (t->lt != 0)
  {
    t = t->lt;
    while (t->rt != 0) t = t->rt;
    return t;
  }
  else
  {
    for (;;)
    {
      if (t->par == 0 || t == t->par->rt)
        return t->par;
      else
        t = t->par;
    }
  }
}


Pix pnodeSplayPQ::seek(pnode  key)
{
  pnodeSplayNode* t = root;
  if (t == 0)
    return 0;

  int comp = pnodeCMP(key, t->item);
  if (comp == 0)
    return Pix(t);

  pnodeSplayNode* dummy = (pnodeSplayNode*)(&_dummy_null);
  pnodeSplayNode* l = dummy;
  pnodeSplayNode* r = dummy;
  dummy->rt = dummy->lt = dummy->par = 0;

  while (comp != 0)
  {
    if (comp > 0)
    {
      pnodeSplayNode* tr = t->rt;
      if (tr == 0)
        break;
      else
      {
        comp = pnodeCMP(key, tr->item);
        if (comp <= 0 || tr->rt == 0)
        {
          l->rt = t; t->par = l;
          l = t;
          t = tr;
          if (comp >= 0)
            break;
        }
        else
        {
          if ((t->rt = tr->lt) != 0) t->rt->par = t;
          tr->lt = t; t->par = tr;
          l->rt = tr; tr->par = l;
          l = tr;
          t = tr->rt;
          comp = pnodeCMP(key, t->item);
        }
      }
    }
    else
    {
      pnodeSplayNode* tl = t->lt;
      if (tl == 0)
        break;
      else
      {
        comp = pnodeCMP(key, tl->item);
        if (comp >= 0 || tl->lt == 0)
        {
          r->lt = t; t->par = r;
          r = t;
          t = tl;
          if (comp <= 0)
            break;
        }
        else
        {
          if ((t->lt = tl->rt) != 0) t->lt->par = t;
          tl->rt = t; t->par = tl;
          r->lt = tl; tl->par = r;
          r = tl;
          t = tl->lt;
          comp = pnodeCMP(key, t->item);
        }
      }
    }
  }
  if ((r->lt = t->rt) != 0) r->lt->par = r;
  if ((l->rt = t->lt) != 0) l->rt->par = l;
  if ((t->lt = dummy->rt) != 0) t->lt->par = t;
  if ((t->rt = dummy->lt) != 0) t->rt->par = t;
  t->par = 0;
  root = t;
  return (comp == 0) ? Pix(t) : 0;
}


Pix pnodeSplayPQ::enq(pnode  item)
{
  ++count;
  pnodeSplayNode* newnode = new pnodeSplayNode(item);
  pnodeSplayNode* t = root;
  if (t == 0)
  {
    root = newnode;
    return Pix(root);
  }

  int comp = pnodeCMP(item, t->item);

  pnodeSplayNode* dummy = (pnodeSplayNode*)(&_dummy_null);
  pnodeSplayNode* l = dummy;
  pnodeSplayNode* r = dummy;
  dummy->rt = dummy->lt = dummy->par = 0;
  
  int done = 0;
  while (!done)
  {
    if (comp >= 0)
    {
      pnodeSplayNode* tr = t->rt;
      if (tr == 0)
      {
        tr = newnode;
        comp = 0; done = 1;
      }
      else
        comp = pnodeCMP(item, tr->item);
        
      if (comp <= 0)
      {
        l->rt = t; t->par = l;
        l = t;
        t = tr;
      }
      else 
      {
        pnodeSplayNode* trr = tr->rt;
        if (trr == 0)
        {
          trr =  newnode;
          comp = 0; done = 1;
        }
        else
          comp = pnodeCMP(item, trr->item);

        if ((t->rt = tr->lt) != 0) t->rt->par = t;
        tr->lt = t; t->par = tr;
        l->rt = tr; tr->par = l;
        l = tr;
        t = trr;
      }
    }
    else
    {
      pnodeSplayNode* tl = t->lt;
      if (tl == 0)
      {
        tl = newnode;
        comp = 0; done = 1;
      }
      else
        comp = pnodeCMP(item, tl->item);

      if (comp >= 0)
      {
        r->lt = t; t->par = r;
        r = t;
        t = tl;
      }
      else
      {
        pnodeSplayNode* tll = tl->lt;
        if (tll == 0)
        {
          tll = newnode;
          comp = 0; done = 1;
        }
        else
          comp = pnodeCMP(item, tll->item);

        if ((t->lt = tl->rt) != 0) t->lt->par = t;
        tl->rt = t; t->par = tl;
        r->lt = tl; tl->par = r;
        r = tl;
        t = tll;
      }
    }
  }
  if ((r->lt = t->rt) != 0) r->lt->par = r;
  if ((l->rt = t->lt) != 0) l->rt->par = l;
  if ((t->lt = dummy->rt) != 0) t->lt->par = t;
  if ((t->rt = dummy->lt) != 0) t->rt->par = t;
  t->par = 0;
  root = t;
  return Pix(root);
}


void pnodeSplayPQ::del(Pix pix)
{
  pnodeSplayNode* t = (pnodeSplayNode*)pix;
  if (t == 0) return;

  pnodeSplayNode* p = t->par;

  --count;
  if (t->rt == 0)
  {
    if (t == root)
    {
      if ((root = t->lt) != 0) root->par = 0;
    }
    else if (t == p->lt)
    {
      if ((p->lt = t->lt) != 0) p->lt->par = p;
    }
    else
      if ((p->rt = t->lt) != 0) p->rt->par = p;
  }
  else
  {
    pnodeSplayNode* r = t->rt;
    pnodeSplayNode* l = r->lt;
    for(;;)
    {
      if (l == 0)
      {
        if (t == root)
        {
          root = r;
          r->par = 0;
        }
        else if (t == p->lt) 
        {
          p->lt = r;
          r->par = p;
        }
        else
        {
          p->rt = r;
          r->par = p;
        }
        if ((r->lt = t->lt) != 0) r->lt->par = r;
        break;
      }
      else
      {
        if ((r->lt = l->rt) != 0) r->lt->par = r;
        l->rt = r; r->par = l;
        r = l;
        l = l->lt;
      }
    }
  }
  delete t;
}

pnode& pnodeSplayPQ::front()
{
  if (root == 0)
    error ("min: empty tree\n");
//  else 
  {
    pnodeSplayNode* t = root;
    pnodeSplayNode* l = root->lt;
    for(;;)
    {
      if (l == 0)
      {
        root = t;
        root->par = 0;
        return root->item;
      }
      else
      {
        if ((t->lt = l->rt) != 0) t->lt->par = t;
        l->rt = t; t->par = l;
        t = l;
        l = l->lt;
      }
    }
  }
}

void pnodeSplayPQ::del_front()
{
  if (root != 0)
  {
    --count;
    pnodeSplayNode* t = root;
    pnodeSplayNode* l = root->lt;
    if (l == 0)
    {
      if ((root = t->rt) != 0) root->par = 0;
      delete t;
    }
    else
    {
      for(;;)
      {
        pnodeSplayNode* ll = l->lt;
        if (ll == 0)
        {
          if ((t->lt = l->rt) != 0) t->lt->par = t;
          delete l;
          break;
        }
        else
        {
          pnodeSplayNode* lll = ll->lt;
          if (lll == 0)
          {
            if ((l->lt = ll->rt) != 0) l->lt->par = l;
            delete ll;
            break;
          }
          else
          {
            t->lt = ll; ll->par = t;
            if ((l->lt = ll->rt) != 0) l->lt->par = l;
            ll->rt = l; l->par = ll;
            t = ll;
            l = lll;
          }
        }
      }
    }
  }
}

pnode pnodeSplayPQ::deq()
{
  if (root == 0)
    error("deq: empty tree");
//  else
  {
    --count;
    pnodeSplayNode* t = root;
    pnodeSplayNode* l = root->lt;
    if (l == 0)
    {
      if ((root = t->rt) != 0) root->par = 0;
      pnode res = t->item;
      delete t;
      return res;
    }
    else
    {
      for(;;)
      {
        pnodeSplayNode* ll = l->lt;
        if (ll == 0)
        {
          if ((t->lt = l->rt) != 0) t->lt->par = t;
          pnode res = l->item;
          delete l;
          return res;
        }
        else
        {
          pnodeSplayNode* lll = ll->lt;
          if (lll == 0)
          {
            if ((l->lt = ll->rt) != 0) l->lt->par = l;
            pnode res = ll->item;
            delete ll;
            return res;
          }
          else
          {
            t->lt = ll; ll->par = t;
            if ((l->lt = ll->rt) != 0) l->lt->par = l;
            ll->rt = l; l->par = ll;
            t = ll;
            l = lll;
          }
        }
      }
    }
  }
}


void pnodeSplayPQ::_kill(pnodeSplayNode* t)
{
  if (t != 0)
  {
    _kill(t->lt);
    _kill(t->rt);
    delete t;
  }
}


pnodeSplayNode* pnodeSplayPQ::_copy(pnodeSplayNode* t)
{
  if (t != 0)
  {
    pnodeSplayNode* l = _copy(t->lt);
    pnodeSplayNode* r = _copy(t->rt);
    pnodeSplayNode* x = new pnodeSplayNode(t->item, l, r);
    if (l != 0) l->par = x;
    if (r != 0) r->par = x;
    return x;
  }
  else 
    return 0;
}

int pnodeSplayPQ::OK()
{
  int v = 1;
  if (root == 0) 
    v = count == 0;
  else
  {
    int n = 1;
    pnodeSplayNode* trail = leftmost();
    pnodeSplayNode* t = succ(trail);
    while (t != 0)
    {
      ++n;
      v &= pnodeCMP(trail->item, t->item) < 0;
      trail = t;
      t = succ(t);
    }
    v &= n == count;
  }
  if (!v) error("invariant failure");
  return v;
}
