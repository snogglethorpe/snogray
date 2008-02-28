// local-surface.cc -- Transformed surface
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

#include "local-surface.h"


using namespace snogray;


LocalSurface::LocalSurface (const Ref<const Material> &mat,
			    const Xform &local_to_world_xform)
  : Surface (mat),
    local_to_world (local_to_world_xform),
    world_to_local (local_to_world_xform.inverse ())
{ }


// Return a bounding box for this surface.
//
// This just returns a bounding box surrounding a 2x2x2 cube from
// (-1,-1,-1) to (1,1,1) in the local coordinate system, as that is
// an appropriate bounding box for many subclasses of LocalSurface.
//
BBox
LocalSurface::bbox () const
{
  return local_to_world (BBox (Pos (-1, -1, -1), Pos (1, 1, 1)));
}
