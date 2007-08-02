// local-surface.cc -- Transformed surface
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "local-surface.h"


using namespace snogray;


LocalSurface::LocalSurface (const Material *mat,
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
