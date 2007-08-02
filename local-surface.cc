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
