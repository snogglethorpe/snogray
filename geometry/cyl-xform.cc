// cyl-xform.cc -- Helper functions for making transformations
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

#include "cyl-xform.h"


using namespace snogray;


// Return a transformation that will transform from a canonical
// coordinate system to one  with the given origin/axis/radius.
//
Xform
snogray::cyl_xform (const Pos &origin, const Vec &axis, const Vec &radius,
		    dist_t height)
{
  Vec az = axis.unit ();
  Vec ax = radius.unit ();
  Vec ay = cross (ax, az);

  dist_t radius_len = radius.length ();

  Xform xf;
  xf.scale (radius_len, radius_len, height);
  xf.to_basis (ax, ay, az);
  xf.translate (Vec (origin));
  return xf;
}

// This is similar, but chooses an arbitrary rotation about AXIS.
//
Xform
snogray::cyl_xform (const Pos &origin, const Vec &axis,
		    dist_t radius, dist_t height)
{
  Vec az = axis.unit ();
  Vec ax = axis.perpendicular ().unit ();
  Vec ay = cross (az, ax);

  Xform xf;
  xf.scale (radius, radius, height);
  xf.to_basis (ax, ay, az);
  xf.translate (Vec (origin));
  return xf;
}
