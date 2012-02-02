// tessel-sphere.cc -- Sphere tessellation
//
//  Copyright (C) 2005-2008, 2012  Miles Bader <miles@gnu.org>
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

#include "tessel-sphere.h"


using namespace snogray;


dist_t
SphereTesselFun::sample_resolution (Tessel::err_t max_err) const
{
  return sqrt (2 * max_err - max_err * max_err);
}

// Define the initial basis edges in TESSEL.
//
void
SphereTesselFun::define_basis (Tessel &tessel) const
{
  // Define our basis.  We use a diamond shape with the pointy ends at the
  // poles.

  const Vertex *p1   = add_vertex (tessel, 0, 1);
  const Vertex *p2   = add_vertex (tessel, 0, 0);
  const Vertex *mid1 = add_vertex (tessel, 0, 0.5f);
  const Vertex *mid2 = add_vertex (tessel, 1.f / 3.f, 0.5f);
  const Vertex *mid3 = add_vertex (tessel, 2.f / 3.f, 0.5f);
  
  add_cell (tessel, p1, mid1, mid2);
  add_cell (tessel, p1, mid2, mid3);
  add_cell (tessel, p1, mid3, mid1);
  
  add_cell (tessel, p2, mid2, mid1);
  add_cell (tessel, p2, mid3, mid2);
  add_cell (tessel, p2, mid1, mid3);
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
SphereTesselFun::midpoint (Tessel &tessel,
			   const Tessel::Vertex *tvert1,
			   const Tessel::Vertex *tvert2)
  const
{
  const Vertex *vert1 = static_cast<const Vertex *>(tvert1);
  const Vertex *vert2 = static_cast<const Vertex *>(tvert2);

  param_t u1 = vert1->u, v1 = vert1->v;
  param_t u2 = vert2->u, v2 = vert2->v;

  // If either vertex is at a "pole" (v = 0 or v = 1), align its
  // u-value with the u-value of the other vertex, so that the
  // resulting midpoint makes sense (we can freely do this because
  // at a pole, the u-value is meaningless).
  //
  if (v1 >= 1 - Eps || v1 <= Eps)
    u1 = u2;
  else if (v2 >= 1 - Eps || v2 <= Eps)
    u2 = u1;

  param_t u = wrapped_midpoint (u1, u2);
  param_t v = (v1 + v2) / 2;

  return add_vertex (tessel, u, v);
}

// Return the surface position corresponding to the parameters U, V.
//
Pos
SphereTesselFun::surface_pos (param_t u, param_t v) const
{
  coord_t theta = u * coord_t (2 * PI);
  coord_t phi = (v - coord_t (0.5)) * coord_t (PI);
  coord_t cos_phi = cos (phi);
  return Pos (-cos (theta) * cos_phi, sin (theta) * cos_phi, sin (phi));
}

// Return the surface normal of VERTEX.
//
// The result need not be normalized (it's the caller's
// responsibility to do so).
//
Vec
SphereTesselFun::vertex_normal (const Vertex &vertex) const
{
  return Vec (SphereTesselFun::surface_pos (vertex.u, vertex.v));
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
