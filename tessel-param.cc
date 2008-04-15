// tessel-param.cc -- Tessellation of parametric surfaces
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

#include <cstdlib>

#include "rand.h"

#include "tessel-param.h"

using namespace snogray;



ParamTesselFun::ParamTesselFun (const Xform &_xform)
  : xform (_xform), reversed_handedness (xform.reverses_handedness ())
{ }

// The size of vertex objects used by this Function (which should be a
// subclass of Tessel::Vertex).
//
size_t
ParamTesselFun::vertex_size () const
{
  return sizeof (Vertex);
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
ParamTesselFun::midpoint (Tessel &tessel,
			  const Tessel::Vertex *tvert1,
			  const Tessel::Vertex *tvert2)
  const
{
  const Vertex *vert1 = static_cast<const Vertex *>(tvert1);
  const Vertex *vert2 = static_cast<const Vertex *>(tvert2);

  return
    add_vertex (tessel, (vert1->u + vert2->u) / 2, (vert1->v + vert2->v) / 2);
}

// Add normal vectors for the vertices in the list from VERTICES_BEG
// to VERTICES_END, to NORMALS.
//
void ParamTesselFun::get_vertex_normals (
		       LinkedList<Tessel::Vertex>::iterator vertices_beg,
		       LinkedList<Tessel::Vertex>::iterator vertices_end,
		       std::vector<SVec> &normals)
  const
{
  Xform norm_xform = xform.inverse ().transpose ();

  for (LinkedList<Tessel::Vertex>::iterator vi = vertices_beg;
       vi != vertices_end; vi++)
    {
      const Vertex &vert = static_cast<const Vertex &>(*vi);
      normals.push_back (SVec (vertex_normal (vert) * norm_xform).unit ());
    }
}

// Add UV values for the vertices in the list from VERTICES_BEG to
// VERTICES_END, to UVS.
//
void
ParamTesselFun::get_vertex_uvs (
	       LinkedList<Tessel::Vertex>::iterator vertices_beg,
	       LinkedList<Tessel::Vertex>::iterator vertices_end,
	       std::vector<UV> &uvs)
  const
{
  for (LinkedList<Tessel::Vertex>::iterator vi = vertices_beg;
       vi != vertices_end; vi++)
    {
      const Vertex &vert = static_cast<const Vertex &>(*vi);
      uvs.push_back (UV (vert.u, vert.v));
    }
}



// Add to TESSEL, and return, a vertex with parameter values U and V.
// The position of the new vertex is automatically calculated using the
// `surface_pos' method.
//
ParamTesselFun::Vertex *
ParamTesselFun::add_vertex (Tessel &tessel, param_t u, param_t v) const
{
  Pos pos = surface_pos (u, v) * xform;
  Vertex *vert = new (alloc_vertex (tessel)) Vertex (u, v, pos);
  Tessel::Function::add_vertex (tessel, vert);
  return vert;
}


// Sphere tessellation

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
  
  add_cell (tessel, p1, mid2, mid1);
  add_cell (tessel, p1, mid3, mid2);
  add_cell (tessel, p1, mid1, mid3);
  
  add_cell (tessel, p2, mid1, mid2);
  add_cell (tessel, p2, mid2, mid3);
  add_cell (tessel, p2, mid3, mid1);
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
  coord_t theta = u * 2 * PIf;
  coord_t phi = (v - 0.5f) * PIf;

  coord_t sin_theta = sin (theta), cos_theta = cos (theta);
  coord_t sin_phi = sin (phi), cos_phi = cos (phi);

  return Pos (cos_theta * cos_phi, sin_theta * cos_phi, sin_phi);
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


// sinc (sin x / x) tessellation

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

  add_cell (tessel, c2, c1, mid);
  add_cell (tessel, c3, c2, mid);
  add_cell (tessel, c1, c3, mid);
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
  return Pos (cos (theta) * v, sin (theta) * v, sinc);
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
  return Vec (cos (theta) * norm_x, sin (theta) * norm_x, norm_y);
}


// Torus tessellation

TorusTesselFun::TorusTesselFun (dist_t _r2, const Xform &_xform)
  : ParamTesselFun (_xform), r1 (1 - _r2), r2 (_r2)
{
  // There's an annoying singularity if r1 == r2, so very slightly
  // perturb r1 and r2 in that case.
  //
  if (r1 == r2)
    {
      r1 -= 0.0001;
      r2 += 0.0001;
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
	add_cell (tessel, verts[r][v], verts[r][next_v], verts[next_r][next_v]);
	add_cell (tessel, verts[r][v], verts[next_r][next_v], verts[next_r][v]);
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

  return Pos (cos (theta) * x_offs, sin (theta) * x_offs, y_offs);
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
  return Vec (cos (theta) * x_norm, sin (theta) * x_norm, y_norm);
}


// arch-tag: e33bedf6-51a0-4e6b-a25b-7238ee36da1a
