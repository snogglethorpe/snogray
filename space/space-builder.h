// space-builder.h -- Builder for Space objects
//
//  Copyright (C) 2007, 2009, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_SPACE_BUILDER_H
#define SNOGRAY_SPACE_BUILDER_H

#include <vector>

#include "util/unique-ptr.h"
#include "surface/surface.h"


namespace snogray {

class Space;


// A class used for building a Space object.
//
class SpaceBuilder
{
public:

  virtual ~SpaceBuilder() { }

  // Add SURFACE to the space being built.
  //
  virtual void add (const Surface *surface) = 0;

  // Return a space containing the objects added through this builder.
  //
  // Note that this can only be done once; after calling this method, the
  // builder should be considered "used" (for instance, it may have
  // transfered some resources to the space object), and the only valid
  // operation on it is to destroy it.
  //
  virtual const Space *make_space () = 0;
};


// This class makes a SpaceBuilder object.
//
class SpaceBuilderFactory
{
public:

  virtual ~SpaceBuilderFactory () { }

  // Return a new space containing SURFACE.
  //
  const Space *make_space (const Surface &surface) const
  {
    UniquePtr<SpaceBuilder> space_builder (make_space_builder ());

    surface.add_to_space (*space_builder);

    return space_builder->make_space ();
  }

  // Return a new SpaceBuilder object.
  //
  virtual SpaceBuilder *make_space_builder () const = 0;
};


}

#endif // SNOGRAY_SPACE_BUILDER_H
