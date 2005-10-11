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

#include "tuple3.h"

namespace Snogray {

class Vec : public Tuple3
{
public:

  Vec (dist_t _x = 0, dist_t _y = 0, dist_t _z = 0) : Tuple3 (_x, _y, _z) { }
  Vec (const Vec &vec) : Tuple3 (vec.x, vec.y, vec.z) { }

  // Allow easy down-casting for sharing code
  //
  Vec (const Tuple3 &tuple) : Tuple3 (tuple) { }

  bool null () const { return x == 0 && y == 0 && z == 0; }

  Vec operator* (const float scale) const
  {
    return Vec (x * scale, y * scale, z * scale);
  }
  Vec operator/ (const float denom) const
  {
    return Vec (x / denom, y / denom, z / denom);
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
  // This gets used in average normals
  void operator/= (const float denom)
  {
    x /= denom; y /= denom; z /= denom;
  }

  Vec operator* (const Transform &xform) const
  {
    return Vec (Tuple3::operator* (xform));
  }
  const Vec &operator*= (const Transform &xform)
  {
    Vec temp = *this * xform;
    *this = temp;
    return *this;
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

  // Return this vector reflected around NORMAL
  //
  Vec reflection (const Vec &normal) const
  {
    // Rr =  Ri - 2 N (Ri . N)

    return  *this - normal * dot (normal) * 2;
  }

  // Return this vector refracted through a medium transition across a
  // surface with normal NORMAL.  IOR_IN and IOR_OUT are the indices of
  // refraction for the incoming and outgoing media.
  //
  Vec refraction (const Vec &normal, float ior_in, float ior_out) const
  {
    // From:
    //
    //   Heckbert, Paul S., Pat Hanrahan, "Beam Tracing Polygonal Objects,",
    //   _Computer Graphics (SIGGRAPH '84 Proceedings)_, vol. 18, no. 3,
    //   July 1984, pp. 119-127.
    // 

    float ior_ratio = ior_in / ior_out;

    float c1 = -dot (normal);
    float c2_sq = 1 - (ior_ratio * ior_ratio) * (1 - c1 * c1);

    if (c2_sq < -Eps)
      return Vec (0, 0, 0);	// Total internal reflection

    if (c2_sq < 0)
      c2_sq = 0;

    return (*this * ior_ratio) + (normal * (ior_ratio * c1 - sqrt (c2_sq)));
  }
};

static inline Vec
operator* (float scale, const Vec &vec)
{
  return vec * scale;
}

extern std::ostream& operator<< (std::ostream &os, const Vec &vec);

}

#endif /* __VEC_H__ */

// arch-tag: f86f6a3f-def9-477b-84a0-0935f0b76e9b
