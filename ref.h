// ref.h -- Reference-counting framework
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __REF_H__
#define __REF_H__


namespace snogray {


// Superclass for subclasses which wish to be reference-counted.
//
class RefCounted
{
public:

  RefCounted (int initial_ref_count = 0) : ref_count (initial_ref_count) { }
  virtual ~RefCounted () { }

  void ref () { ++ref_count; }
  void deref () { if (--ref_count <= 0) { delete this; } }

  mutable int ref_count;
};


// A reference to a ref-counted object, which should implement the
// RefCounted protocol.
//
// Adds a reference to a ref-counted object when initialized/assigned with
// it, and removes the reference when destroyed (or because a new object
// has been assigned), possibly freeing the previously referenced object.
//
template<class T>
class Ref
{
public:

  Ref () : obj (0) { }
  Ref (T *_obj) : obj (_obj) { if (obj) obj->ref (); }
  Ref (const Ref &ref) : obj (ref.obj) { if (obj) obj->ref (); }
  ~Ref () { if (obj) obj->deref (); }

  T &operator* () const { return *obj; }
  T *operator-> () const { return obj; }

  operator bool () const { return !!obj; }

  //operator T* () const { return obj; }

  Ref &operator= (T *new_obj)
  {
    if (obj != new_obj)
      {
	if (obj)
	  obj->deref ();
	obj = new_obj;
	if (obj)
	  obj->ref ();
      }
    return *this;
  }
  Ref &operator= (const Ref &ref)
  {
    return operator= (ref.obj);
  }

private:

  T *obj;
};


}

#endif // __REF_H__


// arch-tag: 6f4ae6fe-3aa0-4575-b051-853e7ea49d32
