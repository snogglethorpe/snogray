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
#include "local-surface.h"


namespace snogray {


// This is a mixture of Primitive and LocalSurface.
//
class LocalPrimitive : public LocalSurface
{
public:

  LocalPrimitive (const Ref<const Material> &mat,
		  const Xform &local_to_world_xform)
    : LocalSurface (local_to_world_xform), _material (mat)
  { }

  Ref<const Material> _material;
};


}


#endif // __LOCAL_PRIMITIVE_H__
