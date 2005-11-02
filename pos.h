// pos.h -- Position datatype
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __POS_H__
#define __POS_H__

#include "vec.h"

namespace Snogray {

class Pos : public Tuple3 
{
public:

  Pos (coord_t _x = 0, coord_t _y = 0, coord_t _z = 0) : Tuple3 (_x, _y, _z) { }
  Pos (const Pos &pos) : Tuple3 (pos.x, pos.y, pos.z) { }

  // Allow easy down-casting for sharing code
  //
  Pos (const Tuple3 &tuple) : Tuple3 (tuple) { }

  Pos operator+ (const Vec &v) const
  {
    return Pos(x + v.x, y + v.y, z + v.z);
  }
  Vec operator- (const Pos &p2) const
  {
    return Vec(x - p2.x, y - p2.y, z - p2.z);
  }

  void operator+= (const Vec &v2)
  {
    x += v2.x; y += v2.y; z += v2.z;
  }
  void operator-= (const Vec &v2)
  {
    x -= v2.x; y -= v2.y; z -= v2.z;
  }

  // Useful for calculating averages
  //
  Pos operator/ (const float denom) const
  {
    return Pos (x / denom, y / denom, z / denom);
  }

  Pos operator* (const Transform &xform) const
  {
    return Pos (Tuple3::operator* (xform));
  }
  const Pos &operator*= (const Transform &xform)
  {
    Pos temp = *this * xform;
    *this = temp;
    return *this;
  }

  dist_t dist (const Pos &p2) const
  {
    return (*this - p2).length ();
  }

  Pos midpoint (const Pos &p2) const
  {
    return Pos((x + p2.x) / 2, (y + p2.y) / 2, (z + p2.z) / 2);
  }
};

extern std::ostream& operator<< (std::ostream &os, const Snogray::Pos &pos);

}

#endif /* __POS_H__ */

// arch-tag: b1fbd699-066c-42c8-9d21-587c24b92f8d
