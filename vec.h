// vec.h -- Vector datatype
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __VEC_H__
#define __VEC_H__

#include <cmath>
#include <fstream>

#include "space.h"

class Vec : public Space::Tuple3 {
public:
  typedef Space::dist_t dist_t;

  Vec (dist_t _x = 0, dist_t _y = 0, dist_t _z = 0) : Tuple3 (_x, _y, _z) { }
  Vec (const Vec &vec) : Tuple3 (vec.x, vec.y, vec.z) { }

  Vec operator* (const float scale) const
  {
    return Vec (x * scale, y * scale, z * scale);
  }
  Vec operator+ (const Vec &v2) const
  {
    return Vec (x + v2.x, y + v2.y, z + v2.z);
  }
  Vec operator- (const Vec &v2) const
  {
    return Vec (x - v2.x, y - v2.y, z - v2.z);
  }
  Vec operator- () const
  {
    return Vec (-x, -y, -z);
  }

  void operator+= (const Vec &v2)
  {
    x += v2.x; y += v2.y; z += v2.z;
  }
  void operator-= (const Vec &v2)
  {
    x -= v2.x; y -= v2.y; z -= v2.z;
  }

  dist_t dot (const Vec &v2) const
  {
    return x * v2.x + y * v2.y + z * v2.z;
  }
  dist_t length_squared () const
  {
    return x * x + y * y + z * z;
  }
  dist_t length () const
  {
    return sqrtf (x * x + y * y + z * z);
  }

  Vec unit () const
  {
    dist_t len = length ();
    if (len == 0)
      return Vec (0, 0, 0);
    else
      return Vec (x / len, y / len, z / len);
  }

  Vec cross (const Vec &vec2) const
  {
    return Vec (y*vec2.z - z*vec2.y, z*vec2.x - x*vec2.z, x*vec2.y - y*vec2.x);
  }
};

static inline Vec
operator* (float scale, const Vec &vec)
{
  return vec * scale;
}

extern std::ostream& operator<< (std::ostream &os, const Vec &vec);

#endif /* __VEC_H__ */

// arch-tag: f86f6a3f-def9-477b-84a0-0935f0b76e9b
