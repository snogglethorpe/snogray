// surface-group.cc -- Group of surfaces
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "space-builder.h"

#include "surface-group.h"


using namespace snogray;


// Add this (or some other) surfaces to the space being built by
// SPACE_BUILDER.
//
void
SurfaceGroup::add_to_space (SpaceBuilder &space_builder) const
{
  space_builder.add (surfaces);
}
