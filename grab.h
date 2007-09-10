// grab.h -- Temporarily acquired reference to an object from a pool
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __GRAB_H__
#define __GRAB_H__

#include "pool.h"


namespace snogray {


// A "grab" is a reference to an object from a pool of objects.
// When the grab is destroyed, the object is returned to the pool.
//
template<class T, class P = Pool<T> >
class Grab
{
public:

  Grab (P &pool) : _pool (pool), _obj (_pool.get ()) { }

  ~Grab () { _pool.put (_obj); }

  T &operator* () const { return *_obj; }
  T *operator-> () const { return _obj; }

private:

  P &_pool;
  T *_obj;
};


}


#endif // __GRAB_H__
