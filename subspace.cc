// subspace.cc -- A surface encapsulated into its own subspace
//
//  Copyright (C) 2007, 2009-2011  Miles Bader <miles@gnu.org>
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
#include "space-builder.h"
#include "snogassert.h"

#include "subspace.h"


using namespace snogray;


Subspace::Subspace (Surface *surf,
		    const SpaceBuilderFactory &space_builder_factory)
  : surface (surf),
    space_builder (space_builder_factory.make_space_builder ())
{ }


// Setup our acceleration structure.
//
void
Subspace::make_space () const
{
  LockGuard guard (make_space_lock);

  if (! space)
    {
      ASSERT (space_builder);

      surface->add_to_space (*space_builder);

      space.reset (space_builder->make_space ());

      space_builder.reset ();
    }
}
