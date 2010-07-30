// local-primitive.h -- Transformed primitive
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

#ifndef __LOCAL_PRIMITIVE_H__
#define __LOCAL_PRIMITIVE_H__


#include "material.h"

#include "primitive.h"
#include "local-xform.h"


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
};


}


#endif // __LOCAL_PRIMITIVE_H__
