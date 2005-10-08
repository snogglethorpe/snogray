#ifndef __POS_H__
#define __POS_H__

#include "space.h"
#include "vec.h"

class Pos : public Space::Tuple3 {
public:
  typedef Space::coord_t coord_t;

  Pos (coord_t _x = 0, coord_t _y = 0, coord_t _z = 0) : Tuple3 (_x, _y, _z) { }
  Pos (const Pos &pos) : Tuple3 (pos.x, pos.y, pos.z) { }

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

  Pos midpoint (const Pos &p2) const
  {
    return Pos((x + p2.x) / 2, (y + p2.y) / 2, (z + p2.z) / 2);
  }
};

extern std::ostream& operator<< (std::ostream &os, const Pos &col);

#endif /* __POS_H__ */

// arch-tag: b1fbd699-066c-42c8-9d21-587c24b92f8d
