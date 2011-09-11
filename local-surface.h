// local-surface.h -- Transformed surface
//
//  Copyright (C) 2007, 2008, 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_LOCAL_SURFACE_H
#define SNOGRAY_LOCAL_SURFACE_H


#include "surface.h"
#include "local-xform.h"


namespace snogray {


// This a base class for surface types which are defined in a local
// coordinate system, and transform back and forth between world-space
// and their local space to do intersection calculations and the like.
//
// It basically just holds two transformations (one for each direction) and
// implements some convenience methods.
//
class LocalSurface : public Surface, public LocalXform
{
public:

  LocalSurface (const Xform &local_to_world_xform)
    : LocalXform (local_to_world_xform)
  { }

  // Return a bounding box for this surface.
  //
  // This just returns a bounding box surrounding a 2x2x2 cube from
  // (-1,-1,-1) to (1,1,1) in the local coordinate system, as that is
  // an appropriate bounding box for many subclasses of LocalSurface.
  //
  virtual BBox bbox () const { return unit_bbox (); }
};


}


#endif // SNOGRAY_LOCAL_SURFACE_H
