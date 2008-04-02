// surface-group.h -- Group of surfaces
//
//  Copyright (C) 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SURFACE_GROUP_H__
#define __SURFACE_GROUP_H__


#include <vector>

#include "surface.h"


namespace snogray {


// A group of surfaces represented as a single composite surface.
//
class SurfaceGroup : public Surface
{
public:

  SurfaceGroup () : Surface (0) { }
  ~SurfaceGroup ();

  // Add SURFACE to this group.
  //
  void add (Surface *surface)
  {
    surfaces.push_back (surface);
    _bbox += surface->bbox ();
  }

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const { return _bbox; }

  // Add this (or some other) surfaces to the space being built by
  // SPACE_BUILDER.
  //
  virtual void add_to_space (SpaceBuilder &space_builder) const;

private:

  // A list of the surfaces in this group.
  //
  std::vector<const Surface *> surfaces;

  // Cached bounding box for the entire group.
  //
  BBox _bbox;
};


}


#endif /* __SURFACE_GROUP_H__ */
