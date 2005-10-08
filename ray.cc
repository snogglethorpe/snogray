#include "ray.h"

std::ostream&
operator<< (std::ostream &os, const Ray &ray)
{
  const Pos &o = ray.origin;
  const Vec &d = ray.dir;
  os << "ray<(" << o.x << ", " << o.y << ", " << o.z
     << ") + (" << d.x << ", " << d.y << ", " << d.z << ") * " << ray.len
     << ">";
  return os;
}

// arch-tag: b1d9b9a4-ee16-451b-b341-20265628a715
