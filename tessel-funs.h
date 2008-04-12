// tessel-funs.h -- Simple interfaces to common tessellations
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

#ifndef __TESSEL_FUNS_H__
#define __TESSEL_FUNS_H__

#include "tessel-param.h"
#include "mesh.h"


namespace snogray {


inline Mesh *
tessel_sphere (const Ref<const Material> &mat,
	       const Pos &origin, dist_t radius, dist_t max_err)
{
  Mesh *mesh = new Mesh (mat);
  SphereTesselFun fun (origin, radius);
  fun.tessellate (mesh, max_err);
  return mesh;
}

// Simple interface to SincTesselFun
//
inline Mesh *
tessel_sinc (const Ref<const Material> &mat,
	     const Pos &origin, dist_t radius, dist_t max_err)
{
  Mesh *mesh = new Mesh (mat);
  SincTesselFun fun (origin, radius);
  fun.tessellate (mesh, max_err);
  return mesh;
}

// Simple interface to TorusTesselFun
//
inline Mesh *
tessel_torus (const Ref<const Material> &mat,
	      const Pos &origin, dist_t radius, dist_t hole_radius,
	      dist_t max_err)
{
  Mesh *mesh = new Mesh (mat);
  TorusTesselFun fun (origin, radius, hole_radius);
  fun.tessellate (mesh, max_err);
  return mesh;
}


}

#endif // __TESSEL_FUNS_H__
