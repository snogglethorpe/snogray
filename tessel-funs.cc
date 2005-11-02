// tessel-funs.cc -- Tessellation of parametric surfaces
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

#include "tessel-funs.h"

using namespace Snogray;


// Sphere tessellation

void
SphereTesselFun::define_basis (Tessel &tessel) const
{
  // We use a diamond shape (8 faces) as a basis

  const Pos &o = origin;
  const dist_t r = radius;

  typedef Tessel::Vertex V;

  const V *top  = add_vertex (tessel, o + Pos (0, r, 0));
  const V *bot  = add_vertex (tessel, o + Pos (0, -r, 0));
  const V *mid1 = add_vertex (tessel, o + Pos (r, 0, 0));
  const V *mid2 = add_vertex (tessel, o + Pos (0, 0, r));
  const V *mid3 = add_vertex (tessel, o + Pos (-r, 0, 0));
  const V *mid4 = add_vertex (tessel, o + Pos (0, 0, -r));

  add_cell (tessel, mid1, top, mid2);
  add_cell (tessel, mid2, top, mid3);
  add_cell (tessel, mid3, top, mid4);
  add_cell (tessel, mid4, top, mid1);

  add_cell (tessel, mid1, bot, mid4);
  add_cell (tessel, mid4, bot, mid3);
  add_cell (tessel, mid3, bot, mid2);
  add_cell (tessel, mid2, bot, mid1);
}

// Return a position on the surface close to POS.  This is the basic
// operation used during tessellation.
//
Pos
SphereTesselFun::surface_pos (const Pos &pos) const
{
  dist_t r = radius;

  if (radius_perturb != 0)
    {
      double rnd = (double (rand()) / double (RAND_MAX));
      r *= (1 + radius_perturb - (rnd * radius_perturb * 2));
    }

  return origin + (pos - origin).unit() * r;
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

// Return a position on the surface close to POS.  This is the basic
// operation used during tessellation.
//
Pos
SincTesselFun::surface_pos (const Pos &pos) const
{
  const Vec offs = pos - origin;
  double u = sqrt (offs.x * offs.x + offs.z * offs.z) * 15 / radius;
  double sinc = (u > -Eps && u < Eps) ? 1.0 : (sin (u) / u);
  return origin + Vec (offs.x, sinc * radius * 0.85, offs.z);
}

void
SincTesselFun::define_basis (Tessel &tessel) const
{
  // We use a pyramid (4 faces) as a basis

  const Pos &o = origin;
  const dist_t r = radius;

  typedef Tessel::Vertex V;

  const V *mid  = add_vertex (tessel, surface_pos (o + Vec ( 0, r,  0)));
  const V *c1   = add_vertex (tessel, surface_pos (o + Vec (-r, 0, -r)));
  const V *c2   = add_vertex (tessel, surface_pos (o + Vec ( r, 0, -r)));
  const V *c3   = add_vertex (tessel, surface_pos (o + Vec ( r, 0,  r)));
  const V *c4   = add_vertex (tessel, surface_pos (o + Vec (-r, 0,  r)));

  add_cell (tessel, c1, mid, c2);
  add_cell (tessel, c2, mid, c3);
  add_cell (tessel, c3, mid, c4);
  add_cell (tessel, c4, mid, c1);
}

dist_t
SincTesselFun::sample_resolution (Tessel::err_t max_err) const
{
  return max_err * 16;
}

// arch-tag: e33bedf6-51a0-4e6b-a25b-7238ee36da1a
