#ifndef __RAY_H__
#define __RAY_H__

#include "pos.h"
#include "vec.h"

class Ray {
public:
  Ray (Pos _origin, Vec _dir) { origin = _origin; dir = _dir.unit (); }
  Ray (Pos _origin, Pos _targ) { Ray (_origin, _targ - _origin); }
  Ray () {}

  /* Returns the end point of the ray.  */
  Pos extension (float scale) const { return origin + dir * scale; }

  Pos origin;
  Vec dir;			// should always be a unit vector
};

#endif /* __RAY_H__ */

// arch-tag: e8ba773e-11bd-4fb2-83b6-ace5f2908aad
