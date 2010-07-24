// surface.cc -- Physical surface
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <stdexcept>

#include "surface.h"
#include "space.h"

using namespace snogray;


// Add this (or some other) surfaces to the space being built by
// SPACE_BUILDER.
//
void
Surface::add_to_space (SpaceBuilder &space_builder) const
{
  space_builder.add (this);
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

// Stubs -- these should be abstract methods, but C++ doesn't allow a
// class with abstract methods to be used in a list/vector, so we just
// signal a runtime error if they're ever called.

static void barf () __attribute__ ((noreturn));
static void
barf ()
{
  throw std::runtime_error ("tried to render abstract surface");
}

const Surface::IsecInfo *
Surface::intersect (Ray &, RenderContext &) const { barf (); }
bool Surface::intersects (const ShadowRay &, RenderContext &) const { barf (); }
BBox Surface::bbox () const { barf (); }


// arch-tag: a62e1854-d7ca-4cb3-a8dc-9be328c53430
