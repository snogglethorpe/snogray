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

#include <stdexcept>

#include "obj.h"
#include "voxtree.h"

using namespace Snogray;

Obj::~Obj () { } // stop gcc bitching

// Return the distance from RAY's origin of the closest intersection of
// this object with RAY, or 0 if there is none.
//
dist_t
Obj::intersection_distance (const Ray &ray) const
{
  return 0;
}

// Given that RAY's origin is known to lie on this object, return the
// distance from RAY's origin to the _next_ closest intersection of this
// object with RAY, or 0 if there is none.  For non-convex objects such
// as triangles, the default implementation which always returns 0 is
// correct.
//
dist_t
Obj::next_intersection_distance (const Ray &ray) const
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
Obj::normal (const Pos &point, const Vec &incoming) const
{
  return 0;
}

BBox
Obj::bbox () const
{
  return BBox (Pos (0,0,0));
}

// Returns the material this object is made from
//
const Material *
Obj::material () const
{
  throw std::runtime_error ("tried to render abstract object");
}

// Add this (or some other ...) objects to SPACE
//
void
Obj::add_to_space (Voxtree &space)
{
  space.add (this);
}

// arch-tag: a62e1854-d7ca-4cb3-a8dc-9be328c53430
