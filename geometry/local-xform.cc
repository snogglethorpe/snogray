// local-xform.cc -- Local/world transformation helper class
//
//  Copyright (C) 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "local-xform.h"


using namespace snogray;


LocalXform::LocalXform (const Xform &local_to_world_xform)
  : local_to_world (local_to_world_xform),
    world_to_local (local_to_world_xform.inverse ())
{ }


// Return a bounding box in world space surrounding a 2x2x2 cube from
// (-1,-1,-1) to (1,1,1) in the local coordinate system (this is an
// appropriate bounding box for many uses).
//
BBox
LocalXform::unit_bbox () const
{
  return local_to_world (BBox (Pos (-1, -1, -1), Pos (1, 1, 1)));
}
