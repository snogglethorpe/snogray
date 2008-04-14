// tessel-funs.cc -- Simple interfaces to common tessellations
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

#include "mesh.h"
#include "tessel-param.h"

#include "tessel-funs.h"


using namespace snogray;


// Helper functions for making transformations

// Return a transformation that will transform from a canonical
// coordinate system to one  with the given origin/axis/radius.
//
Xform cyl_xform (const Pos &origin, const Vec &axis, const Vec &radius,
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
Xform cyl_xform (const Pos &origin, const Vec &axis,
		 dist_t radius, dist_t height)
{
  Vec az = axis.unit ();
  Vec ax = axis.perpendicular ().unit ();
  Vec ay = cross (ax, az);

  Xform xf;
  xf.scale (radius, radius, height);
  xf.to_basis (ax, ay, az);
  xf.translate (Vec (origin));
  return xf;
}


// Simple interface to SphereTesselFun

Mesh *
snogray::tessel_sphere (const Ref<const Material> &mat,
			const Xform &xform, dist_t max_err)
{
  Mesh *mesh = new Mesh (mat);
  SphereTesselFun fun (xform);
  fun.tessellate (mesh, max_err);
  return mesh;
}

Mesh *
snogray::tessel_sphere (const Ref<const Material> &mat,
			const Pos &origin, const Vec &axis, const Vec &radius,
			dist_t max_err)
{
  Xform xform = cyl_xform (origin, axis, radius, radius.length());
  return tessel_sphere (mat, xform, max_err);
}

Mesh *
snogray::tessel_sphere (const Ref<const Material> &mat,
			const Pos &origin, const Vec &axis, dist_t max_err)
{
  dist_t radius = axis.length ();
  Xform xform = cyl_xform (origin, axis, radius, radius);
  return tessel_sphere (mat, xform, max_err);
}


// Simple interface to SincTesselFun

Mesh *
snogray::tessel_sinc (const Ref<const Material> &mat,
		      const Xform &xform, dist_t max_err)
{
  Mesh *mesh = new Mesh (mat);
  SincTesselFun fun (xform);
  fun.tessellate (mesh, max_err);
  return mesh;
}

Mesh *
snogray::tessel_sinc (const Ref<const Material> &mat,
		      const Pos &origin, const Vec &axis, const Vec &radius,
		      dist_t max_err)
{
  Xform xform = cyl_xform (origin, axis, radius, axis.length ());
  return tessel_sinc (mat, xform, max_err);
}

Mesh *
snogray::tessel_sinc (const Ref<const Material> &mat,
		      const Pos &origin, const Vec &axis, dist_t radius,
		      dist_t max_err)
{
  Xform xform = cyl_xform (origin, axis, radius, axis.length ());
  return tessel_sinc (mat, xform, max_err);
}


// Simple interface to TorusTesselFun

Mesh *
snogray::tessel_torus (const Ref<const Material> &mat,
		       const Xform &xform, dist_t hole_frac, dist_t max_err)
{
  Mesh *mesh = new Mesh (mat);
  TorusTesselFun fun (hole_frac, xform);
  fun.tessellate (mesh, max_err);
  return mesh;
}

Mesh *
snogray::tessel_torus (const Ref<const Material> &mat,
		       const Pos &origin, const Vec &axis, const Vec &radius,
		       dist_t max_err)
{
  dist_t radius_len = radius.length ();
  dist_t hole_frac = (radius_len - axis.length () * 2) / radius_len;
  Xform xform = cyl_xform (origin, axis, radius, radius_len);
  return tessel_torus (mat, xform, hole_frac, max_err);
}

Mesh *
snogray::tessel_torus (const Ref<const Material> &mat,
		       const Pos &origin, const Vec &axis, dist_t radius,
		       dist_t max_err)
{
  dist_t hole_frac = (radius - axis.length () * 2) / radius;
  Xform xform = cyl_xform (origin, axis, radius, radius);
  return tessel_torus (mat, xform, hole_frac, max_err);
}
