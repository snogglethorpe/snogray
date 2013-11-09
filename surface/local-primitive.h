// local-primitive.h -- Transformed primitive
//
//  Copyright (C) 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_LOCAL_PRIMITIVE_H
#define SNOGRAY_LOCAL_PRIMITIVE_H


#include "material/material.h"

#include "geometry/local-xform.h"
#include "primitive.h"


namespace snogray {


// This is a mixture of Primitive and LocalSurface.
//
class LocalPrimitive : public Primitive, public LocalXform
{
public:

  LocalPrimitive (const Ref<const Material> &mat,
		  const Xform &local_to_world_xform)
    : Primitive (mat), LocalXform (local_to_world_xform)
  { }

  // Return a bounding box for this surface.
  //
  // This just returns a bounding box surrounding a 2x2x2 cube from
  // (-1,-1,-1) to (1,1,1) in the local coordinate system, as that is
  // an appropriate bounding box for many subclasses of LocalSurface.
  //
  virtual BBox bbox () const { return unit_bbox (); }

  // Transform the geometry of this surface by XFORM.
  //
  virtual void transform (const Xform &xform) { LocalXform::transform (xform); }
};


}


#endif // SNOGRAY_LOCAL_PRIMITIVE_H
