// tuple3.h -- Base class for positions and vectors
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TUPLE3_H__
#define __TUPLE3_H__

#include "coords.h"
#include "matrix4.h"

namespace snogray {

template<typename T>
class Tuple3
{
public:

  Tuple3 (T _x, T _y, T _z) : x (_x), y (_y), z (_z) { }
  Tuple3 () : x (0), y (0), z (0) { }

  // Allow easy "tuple-casting"
  //
  template<typename T2>
  Tuple3 (const Tuple3<T2> &t) : x (t.x), y (t.y), z (t.z) { }

  // Mainly so we can use Tuple3<T> as a map key
  //
  bool operator< (const Tuple3<T> &t) const
  {
    return x < t.x || (x == t.x && (y < t.y || (y == t.y && z < t.z)));
  }
  bool operator== (const Tuple3<T> &t) const
  {
    return x == t.x && y == t.y && z == t.z;
  }
  bool operator!= (const Tuple3<T> &t) const
  {
    return ! operator== (t);
  }

  void operator*= (T denom)
  {
    x *= denom; y *= denom; z *= denom;
  }
  void operator/= (T denom)
  {
    operator*= (1 / denom);
  }

  Tuple3<T> operator* (const Matrix4<T> &xform) const
  {
    return
      Tuple3<T> (
	(  x * xform (0, 0)
	 + y * xform (1, 0)
	 + z * xform (2, 0)
	 +     xform (3, 0)),
	(  x * xform (0, 1)
	 + y * xform (1, 1)
	 + z * xform (2, 1)
	 +     xform (3, 1)),
	(  x * xform (0, 2)
	 + y * xform (1, 2)
	 + z * xform (2, 2)
	 +     xform (3, 2))
	);
  }

  const Tuple3<T> &operator*= (const Matrix4<T> &xform)
  {
    Tuple3<T> temp = *this * xform;
    *this = temp;
    return *this;
  }

  T x, y, z;
};

}

#endif /* __TUPLE3_H__ */

// arch-tag: c796c67c-4af6-4fec-8f17-47bf6128c047
