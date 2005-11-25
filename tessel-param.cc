// tessel-param.cc -- Tessellation of parametric surfaces
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cstdlib>

#include "tessel-param.h"

using namespace Snogray;



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


// Sphere tessellation

// Define the initial basis edges in TESSEL.
//
void
SphereTesselFun::define_basis (Tessel &tessel) const
{
  // Define our basis.  We use a diamond shape with the pointy ends at the
  // poles.

  const Vertex *p1   = add_vertex (tessel, M_PI_2, 0);
  const Vertex *p2   = add_vertex (tessel, -M_PI_2, 0);
  const Vertex *mid1 = add_vertex (tessel, 0, 0);
  const Vertex *mid2 = add_vertex (tessel, 0, 2 * M_PI / 3);
  const Vertex *mid3 = add_vertex (tessel, 0, 4 * M_PI / 3);
  
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

  // If either vertex is at a "pole" ([u = pi/2] or [u = -pi/2]), align its
  // v-value with the v-value of the other vertex, so that the resulting
  // midpoint makes sense (we can freely do this because at a pole, the
  // v-value is meaningless).
  //
  if (u1 >= M_PI_2 - Eps || u1 <= -M_PI_2 + Eps)
    v1 = v2;
  else if (u2 >= M_PI_2 - Eps || u2 <= -M_PI_2 + Eps)
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
    {
      double rnd = (double (rand()) / double (RAND_MAX));
      r *= (1 + radius_perturb - (rnd * radius_perturb * 2));
    }

  double sin_u = sin (u), cos_u = cos (u);
  double sin_v = sin (v), cos_v = cos (v);

  return Pos (cos_v * cos_u * r + origin.x,
	      sin_u 	    * r + origin.y,
	      sin_v * cos_u * r + origin.z);
}

dist_t
SphereTesselFun::sample_resolution (Tessel::err_t max_err) const
{
  return sqrt (2 * radius * max_err - max_err * max_err);
}

// If the subclass can compute vertex normals too, it may use these
// methods to communicate them.

bool
SphereTesselFun::has_vertex_normals () const
{
  return true;
}

Vec
SphereTesselFun::vertex_normal (const Tessel::Vertex &vert) const
{
  return (vert.pos - origin).unit ();
}


// sinc (sin x / x) tessellation

void
SincTesselFun::define_basis (Tessel &tessel) const
{
  // We use a pyramid (4 faces) as a basis

  typedef Tessel::Vertex V;

  const V *mid  = add_vertex (tessel, 0, 0);
  const V *c1   = add_vertex (tessel, 1, 0);
  const V *c2   = add_vertex (tessel, 1, 2 * M_PI / 3);
  const V *c3   = add_vertex (tessel, 1, 4 * M_PI / 3);

  add_cell (tessel, c1, mid, c2);
  add_cell (tessel, c2, mid, c3);
  add_cell (tessel, c3, mid, c1);
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

// Return the surface position corresponding to the parameters U, V.
//
Pos
SincTesselFun::surface_pos (param_t u, param_t v) const
{
  param_t t = u * 5.5 * M_PI;
  double sinc = t < Eps ? 1.0 : sin (t) / t;
  return origin + Vec (cos (v) * u * radius,
		       sinc * radius * 0.85,
		       sin (v) * u * radius);
}

dist_t
SincTesselFun::sample_resolution (Tessel::err_t max_err) const
{
  return max_err * 16;
}

// arch-tag: e33bedf6-51a0-4e6b-a25b-7238ee36da1a
