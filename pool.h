// pool.h -- A pool of objects
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __POOL_H__
#define __POOL_H__

#include <stack>


namespace snogray {


// A "pool" is a group of objects.  Objects may be removed or added to
// the pool, and the object will be notified via a special protocol.
//
// This is useful for keeping around a cache of objects which are
// expensive to create or destroy.
//
template<class T>
class Pool
{
public:

  Pool () { }

  bool empty () const { return _objs.empty (); }

  T *get ()
  {
    if (_objs.empty ())
      return create ();
    else
      {
	T *obj = _objs.top ();
	_objs.pop ();
	obj->acquire ();
	return obj;
      }
  }

  void put (T *obj)
  {
    obj->release ();
    _objs.push (obj);
  }

  T *create ()
  {
    return new T;
  }

private:

  std::stack<T *> _objs;
};


}


#endif // __POOL_H__
