// tessel-param.cc -- Tessellation of parametric surfaces
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
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
  if (can_calc_vertex_normals ())
    for (LinkedList<Tessel::Vertex>::iterator vi = vertices_beg;
	 vi != vertices_end; vi++)
      {
	const Vertex &vert = static_cast<const Vertex &>(*vi);
	normals.push_back (SVec (vertex_normal (vert)));
      }
}

// Return true if this function can calculate vertex normals.
// If so, the `vertex_normal' method will be called to get each vertex's
// normal.  Note that the default is true, so this method need only be
// overridden for subclasses which _can't_ calculate vertex normals.
//
bool
ParamTesselFun::can_calc_vertex_normals () const
{
  return true;
}


// Sphere tessellation

dist_t
SphereTesselFun::sample_resolution (Tessel::err_t max_err) const
{
  return sqrt (2 * radius * max_err - max_err * max_err);
}

// Define the initial basis edges in TESSEL.
//
void
SphereTesselFun::define_basis (Tessel &tessel) const
{
  // Define our basis.  We use a diamond shape with the pointy ends at the
  // poles.

  const Vertex *p1   = add_vertex (tessel, PI/2, 0);
  const Vertex *p2   = add_vertex (tessel, -PI/2, 0);
  const Vertex *mid1 = add_vertex (tessel, 0, 0);
  const Vertex *mid2 = add_vertex (tessel, 0, 2 * PI / 3);
  const Vertex *mid3 = add_vertex (tessel, 0, 4 * PI / 3);
  
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

  // If either vertex is at a "pole" ([u = pi/2] or [u = -pi/2]), align its
  // v-value with the v-value of the other vertex, so that the resulting
  // midpoint makes sense (we can freely do this because at a pole, the
  // v-value is meaningless).
  //
  if (u1 >= PI/2 - Eps || u1 <= -PI/2 + Eps)
    v1 = v2;
  else if (u2 >= PI/2 - Eps || u2 <= -PI/2 + Eps)
    v2 = v1;

  param_t u = angular_midpoint (u1, u2);
  param_t v = angular_midpoint (v1, v2);

  return add_vertex (tessel, u, v);
}

// Return the surface position corresponding to the parameters U, V.
//
Pos
SphereTesselFun::surface_pos (param_t u, param_t v) const
{
  dist_t r = radius;

  if (radius_perturb != 0)
    r *= random (1 - radius_perturb, 1 + radius_perturb);

  coord_t sin_u = sin (u), cos_u = cos (u);
  coord_t sin_v = sin (v), cos_v = cos (v);

  return Pos (cos_v * cos_u * r + origin.x,
	      sin_u 	    * r + origin.y,
	      sin_v * cos_u * r + origin.z);
}

// Return true if this function can calculate vertex normals.
// If so, the `vertex_normal' method will be called to get each vertex's
// normal.  Note that the default is true, so this method need only be
// overridden for subclasses which _can't_ calculate vertex normals.
//
bool
SphereTesselFun::can_calc_vertex_normals () const
{
  return radius_perturb == 0;
}

// Return the surface normal of VERTEX.
//
Vec
SphereTesselFun::vertex_normal (const Vertex &vertex) const
{
  return (vertex.pos - origin).unit ();
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

  typedef Tessel::Vertex V;

  const V *mid  = add_vertex (tessel, 0, 0);
  const V *c1   = add_vertex (tessel, 1, 0);
  const V *c2   = add_vertex (tessel, 1, 2 * PI / 3);
  const V *c3   = add_vertex (tessel, 1, 4 * PI / 3);

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

  // If either vertex is the origin ([u = 0]), align its v-value with the
  // v-value of the other vertex, so that the resulting midpoint makes
  // sense (we can freely do this because at a pole, the v-value is
  // meaningless).
  //
  if (u1 <= Eps)
    v1 = v2;
  else if (u2 <= Eps)
    v2 = v1;

  param_t u = (u1 + u2) / 2;
  param_t v = angular_midpoint (v1, v2);

  return add_vertex (tessel, u, v);
}

#define SINC_X_COMP (5.5 * PI)
#define SINC_Y_COMP (1 / 0.7)

// Return the surface position corresponding to the parameters U, V.
//
Pos
SincTesselFun::surface_pos (param_t u, param_t v) const
{
  param_t t = u * SINC_X_COMP;
  dist_t sinc = t < Eps ? 1.0 : sin (t) / t;
  return origin + Vec (cos (v) * u * radius,
		       sinc * radius * (1 / SINC_Y_COMP),
		       sin (v) * u * radius);
}

// Return the surface normal of VERTEX.
//
Vec
SincTesselFun::vertex_normal (const Vertex &vertex) const
{
  param_t u = vertex.u, v = vertex.v;
  param_t t = u * SINC_X_COMP;
  dist_t deriv = t < Eps ? 0 : (cos (t) / t - sin (t) / (t * t));
  dist_t norm_x = -deriv;
  dist_t norm_y = SINC_Y_COMP / SINC_X_COMP;
  return Vec (cos (v) * norm_x, norm_y, sin (v) * norm_x).unit ();
}


// Torus tessellation

dist_t
TorusTesselFun::sample_resolution (Tessel::err_t max_err) const
{
  dist_t ring_radius = (radius - hole_radius) / 2;
  dist_t r = ring_radius < hole_radius ? ring_radius : hole_radius;
  return sqrt (2 * r * max_err - max_err * max_err);
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
      verts[r][v] = add_vertex (tessel, r * 2 * PI / 3, v * 2 * PI / 3);
  
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

  param_t u = angular_midpoint (u1, u2);
  param_t v = angular_midpoint (v1, v2);

  return add_vertex (tessel, u, v);
}

// Return the surface position corresponding to the parameters U, V.
//
Pos
TorusTesselFun::surface_pos (param_t u, param_t v) const
{
  dist_t ring_radius = (radius - hole_radius) / 2;

  if (radius_perturb != 0)
    ring_radius *= random (1 - radius_perturb, 1 + radius_perturb);

  dist_t x_offs = ring_radius * cos (v) + hole_radius + ring_radius;
  dist_t y_offs = ring_radius * sin (v);

  return origin + Vec (cos (u) * x_offs, y_offs, sin (u) * x_offs);
}

// Return true if this function can calculate vertex normals.
// If so, the `vertex_normal' method will be called to get each vertex's
// normal.  Note that the default is true, so this method need only be
// overridden for subclasses which _can't_ calculate vertex normals.
//
bool
TorusTesselFun::can_calc_vertex_normals () const
{
  return radius_perturb == 0;
}

// Return the surface normal of VERTEX.
//
Vec
TorusTesselFun::vertex_normal (const Vertex &vertex) const
{
  param_t u = vertex.u, v = vertex.v;
  dist_t x_norm = cos (v);
  dist_t y_norm = sin (v);
  return Vec (cos (u) * x_norm, y_norm, sin (u) * x_norm).unit ();
}


// arch-tag: e33bedf6-51a0-4e6b-a25b-7238ee36da1a
