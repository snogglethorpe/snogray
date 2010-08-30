// cylinder.cc -- Cylindrical surface
//
//  Copyright (C) 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "quadratic-roots.h"
#include "intersect.h"

#include "cylinder.h"


using namespace snogray;


// Return a transformation that will transform a canonical cylinder to a
// cylinder with the given base/axis/radius.
//
Xform
Cylinder::xform (const Pos &base, const Vec &axis, float radius)
{
  Vec az = axis.unit ();
  Vec ax = az.perpendicular ();
  Vec zy = cross (ax, az);

  Xform xf;
  xf.translate (Vec (0,0,1));
  xf.scale (radius,radius,axis.length()/2);
  xf.to_basis (ax, zy, az);
  xf.translate (Vec (base));
  return xf;
}


// intersection

// Return true if a cylinder, with radius 1 and height 2, centered at
// the origin and having an axis on the z-axis, is intersected by an
// infinite ray from RAY_ORIGIN in direction RAY_DIR.
//
// When an intersection occurs, the "parametric distance" of the
// intersection is returned in the out-parameter T:  T is the number
// of multiples of RAY_DIR required to reach the intersection point
// from RAY_ORIGIN.  Only intersections with a parameter distance of
// MIN_T or greater are considered.
//
static bool
cylinder_intersects (const Pos &ray_origin, const Vec &ray_dir,
		     dist_t min_t, dist_t &t)
{
  // Cylinder parameters.
  //
  const dist_t radius = 1;
  const coord_t min_z = -1, max_z = 1;

  // Coefficients of the quadratic equation we'll solve.
  //
  dist_t a = ray_dir.x * ray_dir.x + ray_dir.y * ray_dir.y;
  dist_t b = 2 * (ray_dir.x * ray_origin.x + ray_dir.y * ray_origin.y);
  dist_t c = ray_origin.x * ray_origin.x + ray_origin.y * ray_origin.y - radius;

  // Compute intersection points.
  //
  dist_t roots[2];
  unsigned nroots = quadratic_roots (a, b, c, roots);
  for (unsigned i = 0; i < nroots; i++)
    {
      t = roots[i];
      if (t > min_t)
	{
	  coord_t z = ray_origin.z + t * ray_dir.z;
	  if (z >= min_z && z <= max_z)
	    return true;
	}
    }

  return false;
}

// Return true if a cylinder, with radius 1 and height 2, centered at
// the origin and having an axis on the z-axis, is intersected by RAY.
//
// When an intersection occurs, the "parametric distance" of the
// intersection is returned in the out-parameter T:  T is the number
// of multiples of RAY's dir field required to reach the intersection
// point from RAY's origin.
//
static bool
cylinder_intersects (Ray &ray, dist_t &t)
{
  return cylinder_intersects (ray.origin, ray.dir, ray.t0, t) && t < ray.t1;
}



// If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
// to reflect the point of intersection, and return a Surface::IsecInfo
// object describing the intersection (which should be allocated using
// placement-new with CONTEXT); otherwise return zero.
//
const Surface::IsecInfo *
Cylinder::intersect (Ray &ray, RenderContext &context) const
{
  Ray oray = world_to_local (ray);

  dist_t t;
  if (cylinder_intersects (oray, t))
    {
      ray.t1 = t;
      return new (context) IsecInfo (ray, this, oray.extension (t));
    }

  return 0;
}

// Create an Intersect object for this intersection.
//
Intersect
Cylinder::IsecInfo::make_intersect (const Media &media, RenderContext &context)
  const
{
  Pos point = ray.end ();

  Vec onorm (isec_point.x, isec_point.y, 0);
  Vec norm = cylinder->normal_to_world (onorm).unit ();
  Vec t = cylinder->local_to_world (Vec (0, 0, 1)).unit ();
  Vec s = cross (norm, t);

  UV tex_coords (atan2 (isec_point.y, isec_point.x) * INV_PIf * 0.5f + 0.5f,
		 isec_point.z * 0.5f + 0.5f);

  // Calculate partial derivatives of texture coordinates dTds and dTdt,
  // where T is the texture coordinates (for bump mapping).
  //
  UV dTds (INV_PIf * 0.5f, 0), dTdt (0, 0.5f);

  return Intersect (ray, media, context, *cylinder->material,
		    Frame (point, s, t, norm),
		    tex_coords, dTds, dTdt);
}

// Return true if this surface intersects RAY.
//
bool
Cylinder::intersects (const Ray &ray, RenderContext &) const
{
  Ray oray = world_to_local (ray);
  dist_t t;
  return cylinder_intersects (oray, t);
}


// arch-tag: 1a4758de-f640-4ea6-abf2-2626070847e5
