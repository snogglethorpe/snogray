// subspace.cc -- A surface encapsulated into its own subspace
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <memory>

#include "space.h"

#include "subspace.h"


using namespace snogray;


Subspace::~Subspace ()
{
  delete space;
}


// Setup our acceleration structure.
//
void
Subspace::make_space (GlobalTraceState &global) const
{
  std::auto_ptr<SpaceBuilder> space_builder
    (global.space_builder_builder->make_space_builder ());

  surface->add_to_space (*space_builder);

  space = space_builder->make_space ();
}
