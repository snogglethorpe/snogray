// surface.cc -- Physical surface
//
//  Copyright (C) 2005-2013  Miles Bader <miles@gnu.org>
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
#include "space-builder.h"


using namespace snogray;


// Add this (or some other) surfaces to the space being built by
// SPACE_BUILDER.
//
void
Surface::add_to_space (SpaceBuilder &space_builder) const
{
  space_builder.add (this);
}


// arch-tag: a62e1854-d7ca-4cb3-a8dc-9be328c53430
