// ray.cc -- Datatype describing a directional, positioned, line-segment
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "ray.h"

using namespace Snogray;

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
