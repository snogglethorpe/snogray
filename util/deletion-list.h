// deletion-list.h -- List of pointers and deleters
//
//  Copyright (C) 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_DELETION_LIST_H
#define SNOGRAY_DELETION_LIST_H

#include <utility>
#include <vector>


namespace snogray {


// A DeletionList contains a list of pointers and enough information
// to ensure that they can properly be deleted.  When a DeletionList
// is destroyed or DeletionList::clear is called, all the pointers it
// contains are deleted.
//
class DeletionList
{
public:

  DeletionList () { }
  ~DeletionList () { clear (); }

  // Make sure there's no copy-constructor, because that would result
  // in any existing entries being deleted multiple times.
  //
  // To copy a DeletionList, instead use the DeletionList::swap method
  // to swap its contents with another one.
  //
#if __cplusplus >= 201103L
  DeletionList (const DeletionList &) = delete;
  DeletionList (DeletionList &&dl)
    : entries (dl.entries)
  {
    entries.clear ();		// dtors not called
  }
#else
private:
  DeletionList (const DeletionList &);
public:
#endif

  // Add PTR to the list of objects to be deleted.
  //
  template<typename T>
  void add (T *ptr)
  {
    entries.push_back (DelEntry (ptr, deleter<T>));
  }

  // Clear the entries, deleting all objects in it.
  //
  void clear ()
  {
    for (DelList::iterator i = entries.begin (); i != entries.end (); ++i)
      i->second (i->first);

    entries.clear ();
  }

  void swap (DeletionList &dl)
  {
    using std::swap;
    swap (entries, dl.entries);
  }

private:

  typedef std::pair<void *, void (*)(void *)> DelEntry;

  typedef std::vector<DelEntry> DelList;

  // A function that can delete a pointer to a type T which has been
  // cast to void*.
  //
  template<typename T>
  static void deleter (void *ptr)
  {
    delete static_cast<T *> (ptr);
  }

  // The list of pointers to be deleted, and their associated deleter
  // function.
  //
  DelList entries;
};


inline void
swap (DeletionList &dl1, DeletionList &dl2)
{
  dl1.swap (dl2);
}


}


#endif // SNOGRAY_DELETION_LIST_H
