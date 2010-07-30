// local-xform.h -- Local/world transformation helper class
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __LOCAL_XFORM_H__
#define __LOCAL_XFORM_H__

#include "xform.h"
#include "bbox.h"


namespace snogray {


// This a helper class which encompasses some details of
// world-to-local/local-to-world transformation.  It can be used as a
// superclass or as a standalone object as appropriate.
//
// It basically just holds two transformations (one for each direction)
// and some convenience methods.
//
class LocalXform
{
public:

  LocalXform (const Xform &local_to_world_xform);

  // Return the local-space normal NORM transformed to world-space.
  //
  Vec normal_to_world (const Vec &norm) const
  {
    // A normal transformation requires transforming by the transpose of
    // the inverse of the desired transfprmation; since we already have the
    // inverse of the local-to-world available as world_to_local, we can just
    // use "world_to_local.transpose_transform" to do the job.
    //
    return world_to_local.transpose_transform (norm);
  }

  // Return a bounding box in world space surrounding a 2x2x2 cube from
  // (-1,-1,-1) to (1,1,1) in the local coordinate system (this is an
  // appropriate bounding box for many uses).
  //
  BBox unit_bbox () const;

  // Transforms converting between world and local coordinates.
  //
  Xform local_to_world, world_to_local;
};


}

#endif // __LOCAL_XFORM_H__
