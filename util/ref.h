// ref.h -- Reference-counting framework
//
//  Copyright (C) 2005-2008, 2011, 2014  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_REF_H
#define SNOGRAY_REF_H


namespace snogray {


// Superclass for subclasses which wish to be reference-counted.
//
class RefCounted
{
public:

  RefCounted (int initial_ref_count = 0) : ref_count (initial_ref_count) { }
  virtual ~RefCounted () { }

  void ref () const { ++ref_count; }
  void deref () const { if (--ref_count <= 0) { delete this; } }

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

  template<class T2>
  Ref (const Ref<T2> &ref) : obj (ref.ptr ()) { if (obj) obj->ref (); }

  ~Ref () { if (obj) obj->deref (); }

  T &operator* () const { return *obj; }
  T *operator-> () const { return obj; }

  operator bool () const { return !!obj; }

  // In C++, it's more usual to simply use the reference object in a
  // boolean context (e.g., "if (! ref) ..."), but Ref::null is provided
  // as a convenience to Lua, where doing that is not possible.
  //
  bool null () const { return !obj; }

  // Return a pointer to the referenced object; it may be null.
  //
  T *ptr () const { return obj; }

  //operator T* () const { return obj; }

  template<class T2>
  Ref &operator= (T2 *new_obj)
  {
    if (new_obj)
      new_obj->ref ();
    if (obj)
      obj->deref ();
    obj = new_obj;
    return *this;
  }

  Ref &operator= (const Ref &ref) { return operator= (&*ref); }

  template<class T2>
  Ref &operator= (const Ref<T2> &ref) { return operator= (&*ref); }

private:

  T *obj;
};


}

#endif // SNOGRAY_REF_H


// arch-tag: 6f4ae6fe-3aa0-4575-b051-853e7ea49d32
