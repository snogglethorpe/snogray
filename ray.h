#ifndef __RAY_H__
#define __RAY_H__

#include "pos.h"
#include "vec.h"

class Ray {
public:
  Ray (Pos _origin, Vec _dir) { origin = _origin; dir = _dir; }
  Ray (Pos _origin, Pos _targ) { origin = _origin; dir = _targ - _origin; }
  Ray () {}

  Pos origin;
  Vec dir;
};

#endif /* __RAY_H__ */

// arch-tag: e8ba773e-11bd-4fb2-83b6-ace5f2908aad
