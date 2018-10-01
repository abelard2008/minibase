//
// dlist.h      d is for destructive
//
// By Stephen Harris 1994.  Adapted from code by David M. Koscinski.
//
// $Id: dlist.h,v 1.1 1996/02/11 04:25:40 luke Exp $

#ifndef dlist_h
#define dlist_h

#include "da_types.h"
#include "listbase.h"

template <class T>
class DList: public ListBase<T>
{
public:
	DList(): ListBase<T>() { /* empty */ };
	DList(const DList<T> & l);      // copy constructor; copies only
					// the links, the nodes of l are
					// the same as for the
					// resulting list

	DList(const DList<T> * l);	// same as the copy constructor;
					// provided to be consistent with
                                        // class NDList

	~DList() { ListBase<T>::UnLink(); };

	void operator=(const DList<T> * l);

	// Concatenates argument to end of list.  Does not copy data in
	// argument, only pointers.
	void operator+(const DList<T> * l);

        // removes items from list that are also in l.  Does not perform
        // delete on any elements.
        void operator-(const DList<T> * l);

	T * Insert(T * t);  		// t is not copied i.e. -
					// its address is the same
					// in the list as before inserted
	T * InsertBeforeCurr(T * t);
	T * InsertHead(T * t);
	T * InsertTail(T * t);
};


template <class T>
inline DList<T>::DList(const DList<T> & l)
{
	ListBase<T>::head = ListBase<T>::tail = ListBase<T>::curr = NULL;
	ListBase<T>::count = 0;

	ListNode<T> *n;

	for( n=l.head; n; n=n->next )
		Insert(n->data);

	ListBase<T>::curr = ListBase<T>::head;
}



template <class T>
inline DList<T>::DList(const DList<T> * l)
{
	ListBase<T>::head = ListBase<T>::tail = ListBase<T>::curr = NULL;
	ListBase<T>::count = 0;

	ListNode<T> *n;

	for( n=l->head; n; n=n->next )
		Insert(n->data);

	ListBase<T>::curr = ListBase<T>::head;
}




template <class T>
inline void DList<T>::operator=(const DList<T> * l)
{
  ListBase<T>::UnLink();

	ListNode<T> *n;

	for( n=l->head; n; n=n->next )
		Insert(n->data);

	ListBase<T>::curr = ListBase<T>::head;
}


// concatenation
template <class T>
inline void DList<T>::operator+(const DList<T> * l)
{
	ListNode<T> *n;
	 
	for (n = l->head; n; n = n->next)
	  InsertTail(n->data);
}

// subtraction
template <class T>
inline void DList<T>::operator-(const DList<T> * l)
{
  ListNode<T> *n;

  for (n = l->head; n; n = n->next) 
    ListBase<T>::Detach(n->data);
}

// insert after curr node
template <class T>
inline T * DList<T>::Insert(T * t)
{
	ListNode<T> *temp = new ListNode<T>(t);
	if (!temp) FatalError(MEM);

#ifdef debug_messages
	temp->destructiveInsert = TRUE;
#endif

	if( ListBase<T>::count == 0 )
	{
		ListBase<T>::head = temp;
		ListBase<T>::tail = temp;
		temp->next = NULL;
	}
	else if( ListBase<T>::curr == NULL )
	{
      ListBase<T>::tail->next = temp;
		ListBase<T>::tail = temp;
		temp->next = NULL;
	}
	else
	{
		temp->next = ListBase<T>::curr->next;
		ListBase<T>::curr->next = temp;
		if( ListBase<T>::tail == ListBase<T>::curr )
			ListBase<T>::tail = temp;
	}
	ListBase<T>::curr = temp;
	ListBase<T>::count++;

	return ListBase<T>::CurrPtr();
}

// insert before curr node
template <class T>
inline T * DList<T>::InsertBeforeCurr(T * t)
{
	ListNode<T> *ln = ListBase<T>::curr;

	if (ListBase<T>::curr == ListBase<T>::head || ListBase<T>::count == 0) 
        	return InsertHead(t);
	else {	// set curr to preceding node and insert after that one
		for( ListBase<T>::curr = ListBase<T>::head; ListBase<T>::curr->next != ln; ListBase<T>::curr = ListBase<T>::curr->next );
		return Insert(t);
	}
}

// insert at head
template <class T>
inline T * DList<T>::InsertHead(T * t)
{
	ListNode<T> *temp = new ListNode<T>(t);
	if (!temp) FatalError(MEM);

#ifdef debug_messages
	temp->destructiveInsert = TRUE;
#endif

	temp->next = ListBase<T>::head;
	if( ListBase<T>::tail == NULL )
		ListBase<T>::tail = temp;
	ListBase<T>::head = temp;

	ListBase<T>::curr = temp;
	ListBase<T>::count++;

	return ListBase<T>::CurrPtr();
}


// insert at tail
template <class T>
inline T * DList<T>::InsertTail(T * t)
{
	ListNode<T> *temp = new ListNode<T>(t);
	if (!temp) FatalError(MEM);

#ifdef debug_messages
	temp->destructiveInsert = TRUE;
#endif

	if( ListBase<T>::head == NULL )
		ListBase<T>::head = temp;
	else
		ListBase<T>::tail->next = temp;

	ListBase<T>::tail = temp;
	temp->next = NULL;

	ListBase<T>::curr = temp;
	ListBase<T>::count++;

	return ListBase<T>::CurrPtr();
}

#endif
