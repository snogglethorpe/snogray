// tessel-torus.cc -- Torus tessellation
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

#include "tessel-torus.h"


using namespace snogray;


TorusTesselFun::TorusTesselFun (dist_t _r2, const Xform &_xform)
  : ParamTesselFun (_xform), r1 (1 - _r2), r2 (_r2)
{
  // There's an annoying singularity if r1 == r2, so very slightly
  // perturb r1 and r2 in that case.
  //
  if (r1 == r2)
    {
      r1 -= dist_t (0.0001);
      r2 += dist_t (0.0001);
    }
}

dist_t
TorusTesselFun::sample_resolution (Tessel::err_t max_err) const
{
  return sqrt (2 * min (r1, r2) * max_err - max_err * max_err);
}

// Define the initial basis edges in TESSEL.
//
void
TorusTesselFun::define_basis (Tessel &tessel) const
{
  // Define our basis.  We use a triangular "donut", composed of three
  // rings with triangular cross-sections.

  const Vertex *verts[3][3];

  for (unsigned r = 0; r < 3; r++)
    for (unsigned v = 0; v < 3; v++)
      verts[r][v] = add_vertex (tessel, r / 3.f, v / 3.f);
  
  for (unsigned r = 0; r < 3; r++)
    for (unsigned v = 0; v < 3; v++)
      {
	unsigned next_r = (r + 1) % 3;
	unsigned next_v = (v + 1) % 3;
	add_cell (tessel, verts[r][v], verts[next_r][next_v], verts[r][next_v]);
	add_cell (tessel, verts[r][v], verts[next_r][v], verts[next_r][next_v]);
      }
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
TorusTesselFun::midpoint (Tessel &tessel,
			   const Tessel::Vertex *tvert1,
			   const Tessel::Vertex *tvert2)
  const
{
  const Vertex *vert1 = static_cast<const Vertex *>(tvert1);
  const Vertex *vert2 = static_cast<const Vertex *>(tvert2);

  param_t u1 = vert1->u, v1 = vert1->v;
  param_t u2 = vert2->u, v2 = vert2->v;

  param_t u = wrapped_midpoint (u1, u2);
  param_t v = wrapped_midpoint (v1, v2);

  return add_vertex (tessel, u, v);
}

// Return the surface position corresponding to the parameters U, V.
//
Pos
TorusTesselFun::surface_pos (param_t u, param_t v) const
{
  dist_t theta = u * 2 * PIf;
  dist_t phi = v * 2 * PIf;

  dist_t x_offs = r2 * cos (phi) + r1;
  dist_t y_offs = r2 * sin (phi);

  return Pos (-cos (theta) * x_offs, sin (theta) * x_offs, y_offs);
}

// Return the surface normal of VERTEX.
//
// The result need not be normalized (it's the caller's
// responsibility to do so).
//
Vec
TorusTesselFun::vertex_normal (const Vertex &vertex) const
{
  dist_t theta = vertex.u * 2 * PIf;
  dist_t phi = vertex.v * 2 * PIf;
  dist_t x_norm = cos (phi);
  dist_t y_norm = sin (phi);
  return Vec (-cos (theta) * x_norm, sin (theta) * x_norm, y_norm);
}


// Simple interface to TorusTesselFun

Mesh *
snogray::tessel_torus (const Ref<const Material> &mat,
		       const Xform &xform, float r2_frac, dist_t max_err)
{
  Mesh *mesh = new Mesh (mat);
  TorusTesselFun fun (r2_frac, xform);
  fun.tessellate (mesh, max_err);
  return mesh;
}

Mesh *
snogray::tessel_torus (const Ref<const Material> &mat,
		       const Pos &origin, const Vec &axis, const Vec &radius,
		       dist_t max_err)
{
  dist_t radius_len = radius.length ();
  float r2_frac = axis.length () / radius_len;
  Xform xform = cyl_xform (origin, axis, radius, radius_len);
  return tessel_torus (mat, xform, r2_frac, max_err);
}

Mesh *
snogray::tessel_torus (const Ref<const Material> &mat,
		       const Pos &origin, const Vec &axis, dist_t radius,
		       dist_t max_err)
{
  float r2_frac = axis.length() / radius;
  Xform xform = cyl_xform (origin, axis, radius, radius);
  return tessel_torus (mat, xform, r2_frac, max_err);
}
