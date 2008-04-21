// cyl-xform.h -- Helper functions for making transformations
//
//  Copyright (C) 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __CYL_XFORM_H__
#define __CYL_XFORM_H__

#include "xform.h"
#include "pos.h"
#include "vec.h"


namespace snogray {


// Return a transformation that will transform from a canonical
// coordinate system to one  with the given origin/axis/radius.
//
Xform cyl_xform (const Pos &origin, const Vec &axis, const Vec &radius,
		 dist_t height);

// This is similar, but chooses an arbitrary rotation about AXIS.
//
Xform cyl_xform (const Pos &origin, const Vec &axis,
		 dist_t radius, dist_t height);


}

#endif // __CYL_XFORM_H__
