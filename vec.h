#ifndef __VEC_H__
#define __VEC_H__

#include <cmath>

#include "space.h"

class Vec : public Space::Tuple3 {
public:
  typedef Space::dist_t dist_t;

  Vec (dist_t _x, dist_t _y, dist_t _z) { x = _x; y = _y; z = _z; }
  Vec () { x = y = z = 0; }

  Vec (const Vec &vec) { x = vec.x; y = vec.y; z = vec.z; }

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

  Vec cross (const Vec &vec2) const
  {
    return Vec (y * vec2.z - z * vec2.y,
		 z * vec2.x - x * vec2.z,
		 x * vec2.y - y * vec2.x);
  }

  Vec normal () const
  {
    dist_t len = length ();
    if (len == 0)
      return Vec (0, 0, 0);
    else
      return Vec (x / len, y / len, z / len);
  }
};

static inline Vec
operator* (float scale, const Vec &vec)
{
  return vec * scale;
}

#endif /* __VEC_H__ */
// arch-tag: f86f6a3f-def9-477b-84a0-0935f0b76e9b
