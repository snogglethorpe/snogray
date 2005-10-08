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

void
Obj::closest_intersect (Intersect &isec) const
{
}

void
Obj::finish_intersect (Intersect &isec) const
{
}

bool
Obj::intersects (const Ray &ray) const
{
  return false;
}

// arch-tag: a62e1854-d7ca-4cb3-a8dc-9be328c53430
