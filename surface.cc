// surface.cc -- Physical surface
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

#include "surface.h"
#include "voxtree.h"

using namespace Snogray;

Surface::~Surface () { } // stop gcc bitching

// Return the distance from RAY's origin to the closest intersection
// of this surface with RAY, or 0 if there is none.  RAY is considered
// to be unbounded.
//
// NUM is which intersection to return, for non-flat surfaces that may
// have multiple intersections -- 0 for the first, 1 for the 2nd, etc
// (flat surfaces will return failure for anything except 0).
//
dist_t
Surface::intersection_distance (const Ray &ray, unsigned num) const
{
  return 0;
}

// Making the following virtual slows things down measurably, and there
// are no cases where it's needed yet:
//
// bool
// Surface::intersects (const Ray &ray) const
// {
//   Space::dist_t dist = intersection_distance (ray);
//   return dist > 0 && dist < ray.len;
// }

Vec
Surface::normal (const Pos &point, const Vec &incoming) const
{
  return 0;
}

BBox
Surface::bbox () const
{
  return BBox (Pos (0,0,0));
}

// Returns the material this surface is made from
//
const Material *
Surface::material () const
{
  throw std::runtime_error ("tried to render abstract surface");
}

// Add this (or some other ...) surfaces to SPACE
//
void
Surface::add_to_space (Voxtree &space)
{
  space.add (this);
}

// arch-tag: a62e1854-d7ca-4cb3-a8dc-9be328c53430
