// ray.cc -- Datatype describing a directional, positioned, line-segment
//
//  Copyright (C) 2005, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "ray.h"

using namespace snogray;

std::ostream&
snogray::operator<< (std::ostream &os, const Ray &ray)
{
  const Pos &o = ray.origin;
  const Vec &d = ray.dir;
  if (ray.t0 == 0)
    os << "ray<(" << o.x << ", " << o.y << ", " << o.z
       << ") + (" << d.x << ", " << d.y << ", " << d.z
       << ") * " << ray.t1
       << ">";
  else
    os << "ray<(" << o.x << ", " << o.y << ", " << o.z
       << ") + (" << d.x << ", " << d.y << ", " << d.z
       << ") * (" << ray.t0 << " - " << ray.t1 << ")"
       << ">";
  return os;
}

// arch-tag: b1d9b9a4-ee16-451b-b341-20265628a715
