// obj.cc -- Root of object class hierarchy
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "obj.h"

using namespace Snogray;

Obj::~Obj () { } // stop gcc bitching

dist_t
Obj::intersection_distance (const Ray &ray) const
{
  return 0;
}

// Making the following virtual slows things down measurably, and there
// are no cases where it's needed yet:
//
// bool
// Obj::intersects (const Ray &ray) const
// {
//   Space::dist_t dist = intersection_distance (ray);
//   return dist > 0 && dist < ray.len;
// }

Vec
Obj::normal (const Pos &point, const Vec &eye_dir) const
{
  return 0;
}

BBox
Obj::bbox () const
{
  return BBox (Pos (0,0,0));
}

// arch-tag: a62e1854-d7ca-4cb3-a8dc-9be328c53430
