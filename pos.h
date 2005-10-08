#ifndef __POS_H__
#define __POS_H__

#include "space.h"
#include "vec.h"

class Pos : public Space::Tuple3 {
public:
  typedef Space::coord_t coord_t;

  Pos (coord_t _x, coord_t _y, coord_t _z) { x = _x; y = _y; z = _z; }
  Pos () { x = y = z = 0; }

  Pos (const Pos &pos) { x = pos.x; y = pos.y; z = pos.z; }

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
};

#endif /* __POS_H__ */

// arch-tag: b1fbd699-066c-42c8-9d21-587c24b92f8d
