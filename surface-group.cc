// surface-group.cc -- Group of surfaces
//
//  Copyright (C) 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "space-builder.h"

#include "surface-group.h"


using namespace snogray;


SurfaceGroup::~SurfaceGroup ()
{
  for (std::vector<const Surface *>::const_iterator si = surfaces.begin();
       si != surfaces.end(); ++si)
    delete *si;
}



// Add SURFACE to this group.
//
void
SurfaceGroup::add (const Surface *surface)
{
  surfaces.push_back (surface);
  _bbox += surface->bbox ();
}


// Add this (or some other) surfaces to the space being built by
// SPACE_BUILDER.
//
void
SurfaceGroup::add_to_space (SpaceBuilder &space_builder) const
{
  for (std::vector<const Surface *>::const_iterator si = surfaces.begin();
       si != surfaces.end(); ++si)
    (*si)->add_to_space (space_builder);
}


// If this surface, or some part of it, uses any light-emitting
// materials, add appropriate Light objects to LIGHTS.  Any lights
// added become owned by the owner of LIGHTS, and will be destroyed
// when it is.
//
void
SurfaceGroup::add_lights (std::vector<Light *> &lights) const
{
  for (std::vector<const Surface *>::const_iterator si = surfaces.begin();
       si != surfaces.end(); ++si)
    (*si)->add_lights (lights);
}
