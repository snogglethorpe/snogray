#include "vec.h"

class Ray {
public:
  Ray (Point3 _origin, Vec3 _dir) { origin = _origin; dir = _dir; }
  Ray () {}

  Point3 origin;
  Vec3 dir;
};

// arch-tag: e8ba773e-11bd-4fb2-83b6-ace5f2908aad
