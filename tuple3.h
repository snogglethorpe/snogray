// tuple3.h -- Base class for positions and vectors
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TUPLE3_H__
#define __TUPLE3_H__

#include "space.h"
#include "transform.h"

namespace Snogray {

class Tuple3
{
public:

  Tuple3 (coord_t _x = 0, coord_t _y = 0, coord_t _z = 0)
    : x (_x), y (_y), z (_z)
  { }

  // Mainly so we can use Tuple3 as a map key
  //
  bool operator< (const Tuple3 &t) const
  {
    return x < t.x || (x == t.x && (y < t.y || (y == t.y && z < t.z)));
  }
  bool operator== (const Tuple3 &t) const
  {
    return x == t.x && y == t.y && z == t.z;
  }
  bool operator!= (const Tuple3 &t) const
  {
    return ! operator== (t);
  }

  Tuple3 operator* (const Transform &xform) const
  {
    return
      Tuple3 (
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

  const Tuple3 &operator*= (const Transform &xform)
  {
    Tuple3 temp = *this * xform;
    *this = temp;
    return *this;
  }

  coord_t x, y, z;
};  

}

#endif /* __TUPLE3_H__ */

// arch-tag: c796c67c-4af6-4fec-8f17-47bf6128c047
