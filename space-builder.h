// space-builder.h -- Builder for Space objects
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SPACE_BUILDER_H__
#define __SPACE_BUILDER_H__

#include <vector>


namespace snogray {

class Space;
class Surface;


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
class SpaceBuilderBuilder
{
public:

  virtual ~SpaceBuilderBuilder () { }

  // Return a new SpaceBuilder object.
  //
  virtual SpaceBuilder *make_space_builder () const = 0;
};


}

#endif // __SPACE_BUILDER_H__
