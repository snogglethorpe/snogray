// llist.h -- Linked-list wrapper type
//
//  Copyright (C) 2005, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __LLIST_H__
#define __LLIST_H__

namespace snogray {

// This simple linked list class implements "direct linked" objects,
// where the objects in the list each have their own prev/next pointers
// (and thus can only be in one linked-list at a time).  In some
// situations it's a lot less annoying that STL lists.
//
// Usage:  If you want a linked list of `Foo' objects, then have class
// Foo inherit directly from `public LinkedList<Foo>::Node'.  Then you
// can declare the linked list like `LinkedList<Foo>', and append Foo
// objects to it, etc.  When the linked-list is destroyed, any remaining
// list members are also destroyed using delete.
//
template<class T>
class LinkedList
{
public:

  LinkedList () : head (0), tail (0), num_entries (0) { }

  // The superclass from which T should inherit
  //
  class Node
  {
  public:

    Node () : prev (0), next (0) { }

  private:

    friend class LinkedList<T>;
    friend class LinkedList<T>::iterator;

    T *prev, *next;
  };

  // An iterator for T
  //
  class iterator
  {
  public:

    T &operator* () { return *node; }
    T *operator-> () { return node; }

    bool operator== (const iterator &i) const { return node == i.node; }
    bool operator!= (const iterator &i) const { return node != i.node; }
    iterator &operator++ () { if (node) node = node->next; return *this; }
    iterator operator++ (int)
    {
      T *old = node;
      if (node)
	node = node->next;
      return old;
    }

  private:

    friend class LinkedList;

    iterator (T *_node) : node (_node) { }

    T *node;
  };
  friend class iterator;

  T *first () const { return head; }
  T *last () const { return tail; }

  T *append (T *n)
  {
    n->prev = tail;
    if (tail)
      tail->next = n;
    tail = n;

    if (! head)
      head = n;

    num_entries++;

    return n;
  }

  void remove (T *n)
  {
    if (n->next)
      n->next->prev = n->prev;
    else
      tail = n->prev;
    if (n->prev)
      n->prev->next = n->next;
    else
      head = n->next;

    num_entries--;
  }

  T *push (T *n)
  {
    n->prev = 0;
    n->next = head;

    if (head)
      head->prev = n;
    else
      tail = n;

    head = n;

    num_entries++;

    return n;
  }

  T *pop ()
  {
    if (head)
      {
	T *n = head;

	head = n->next;
	if (head)
	  head->prev = 0;
	else
	  tail = 0;

	num_entries--;

	return n;
      }
    else
      return 0;
  }

  bool empty () const { return !head; }
  operator bool () const { return head; }

  unsigned size () const { return num_entries; }

  // iterator support
  //
  iterator begin () const { return iterator (head); }
  iterator end () const { return iterator (0); }

private:

  T *head, *tail;
  unsigned num_entries;
};

}

#endif /* __LLIST_H__ */

// arch-tag: 34227d90-5372-43ec-b4f2-c7b4e8f70601
