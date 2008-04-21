// tessel-sinc.cc -- sinc (sin x / x) tessellation
//
//  Copyright (C) 2005, 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "mesh.h"
#include "cyl-xform.h"

#include "tessel-sinc.h"


using namespace snogray;


dist_t
SincTesselFun::sample_resolution (Tessel::err_t max_err) const
{
  return max_err * 16;
}

void
SincTesselFun::define_basis (Tessel &tessel) const
{
  // We use a pyramid (4 faces) as a basis

  const Vertex *mid  = add_vertex (tessel, 0, 0);
  const Vertex *c1   = add_vertex (tessel, 0, 1);
  const Vertex *c2   = add_vertex (tessel, 1.f / 3.f, 1);
  const Vertex *c3   = add_vertex (tessel, 2.f / 3.f, 1);

  add_cell (tessel, c2, mid, c1);
  add_cell (tessel, c3, mid, c2);
  add_cell (tessel, c1, mid, c3);
}

// Add to TESSEL and return a new vertex which is on this function's
// surface midway between VERT1 and VERT2 (for some definition of
// "midway").  This is the basic operation used during tessellation.
// VERT1 and VERT2 are guaranteed to have come from either the original
// basis defined by `define_basis', or from a previous call to
// `midpoint'; thus it is safe for subclasses to down-cast them to
// whatever Vertex subclass they use.
//
Tessel::Vertex *
SincTesselFun::midpoint (Tessel &tessel,
			   const Tessel::Vertex *tvert1,
			   const Tessel::Vertex *tvert2)
  const
{
  const Vertex *vert1 = static_cast<const Vertex *>(tvert1);
  const Vertex *vert2 = static_cast<const Vertex *>(tvert2);

  param_t u1 = vert1->u, v1 = vert1->v;
  param_t u2 = vert2->u, v2 = vert2->v;

  // If either vertex is the origin (v = 0), align its u-value with
  // the u-value of the other vertex, so that the resulting midpoint
  // makes sense (we can freely do this because at a pole, the
  // u-value is meaningless).
  //
  if (v1 <= Eps)
    u1 = u2;
  else if (v2 <= Eps)
    u2 = u1;

  param_t u = wrapped_midpoint (u1, u2);
  param_t v = (v1 + v2) / 2;

  return add_vertex (tessel, u, v);
}

#define SINC_X_COMP (5.5 * PI)

// Return the surface position corresponding to the parameters U, V.
//
Pos
SincTesselFun::surface_pos (param_t u, param_t v) const
{
  param_t theta = u * 2 * PIf;
  param_t t = v * SINC_X_COMP;
  dist_t sinc = t < Eps ? 1.0 : sin (t) / t;
  return Pos (-cos (theta) * v, sin (theta) * v, sinc);
}

// Return the surface normal of VERTEX.
//
// The result need not be normalized (it's the caller's
// responsibility to do so).
//
Vec
SincTesselFun::vertex_normal (const Vertex &vertex) const
{
  param_t theta = vertex.u * 2 * PIf;
  param_t t = vertex.v * SINC_X_COMP;
  dist_t deriv = t < Eps ? 0 : (cos (t) / t - sin (t) / (t * t));
  dist_t norm_x = -deriv;
  dist_t norm_y = 1 / SINC_X_COMP;
  return Vec (-cos (theta) * norm_x, sin (theta) * norm_x, norm_y);
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
