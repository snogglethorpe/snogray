// local-surface.h -- Transformed surface
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __LOCAL_SURFACE_H__
#define __LOCAL_SURFACE_H__


#include "xform.h"

#include "surface.h"


namespace snogray {


// This a base class for surface types which are defined in a local
// coordinate system, and transform back and forth between world-space
// and their local space to do intersection calculations and the like.
//
// It basically just holds two transformations (one for each direction) and
// implements some convenience methods.
//
class LocalSurface : public Surface
{
public:

  LocalSurface (const Material *mat, const Xform &local_to_world_xform);

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

  // Transforms converting between world and local coordinates.
  //
  Xform local_to_world, world_to_local;
};


}


#endif /* __LOCAL_SURFACE_H__ */
