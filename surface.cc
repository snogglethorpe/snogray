// surface.cc -- Physical surface
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <stdexcept>

#include "surface.h"
#include "space.h"

using namespace snogray;


// Add this (or some other ...) surfaces to SPACE
//
void
Surface::add_to_space (Space &space)
{
  space.add (this);
}

// The "smoothing group" this surface belongs to, or zero if it belongs
// to none.  The smoothing group affects shadow-casting: if two objects
// are in the same smoothing group, they will not be shadowed by
// back-surface shadows from each other; typically all triangles in a
// mesh are in the same smoothing group.
//
const void *
Surface::smoothing_group () const
{
  return 0;
}

// Confirm that this surfaces blocks RAY, which emanates from the
// intersection ISEC.  DIST is the distance between ISEC and the position
// where RAY intersects this surface.
//
bool
Surface::confirm_shadow (const Ray &, dist_t, const Intersect &)
  const
{
  return true;
}

// Stubs -- these should be abstract methods, but C++ doesn't allow a
// class with abstract methods to be used in a list/vector, so we just
// signal a runtime error if they're ever called.

static void barf () __attribute__ ((noreturn));
static void
barf ()
{
  throw std::runtime_error ("tried to render abstract surface");
}

bool Surface::intersect (Ray &, IsecParams &) const { barf (); }
Intersect Surface::intersect_info (const Ray &, const IsecParams &, Trace &) const { barf (); }

BBox Surface::bbox () const { barf (); }


// arch-tag: a62e1854-d7ca-4cb3-a8dc-9be328c53430
