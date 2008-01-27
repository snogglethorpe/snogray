// cylinder.cc -- Cylindrical surface
//
//  Copyright (C) 2007, 2008  Miles Bader <miles@gnu.org>
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
#include "shadow-ray.h"

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


// Return the point of intersection of RAY with a radius 1 cylinder of
// height 2, centered at the origin.  If RAY doesn't intersect, return
// zero, otherwise the return value T is the parametric distance from
// RAY.origin; to calculate the actual intersection point, use
// RAY.origin + T * RAY.dir.
//
static dist_t
cylinder_intersect (Ray &ray)
{
  const Pos &o = ray.origin;
  const Vec &d = ray.dir;

  // Coefficients of the quadratic equation we'll solve.
  //
  dist_t a = d.x * d.x + d.y * d.y;
  dist_t b = 2 * (d.x * o.x + d.y * o.y);
  dist_t c = o.x * o.x + o.y * o.y - 1 /* radius */;

  // Compute intersection points.
  //
  dist_t roots[2];
  unsigned nroots = quadratic_roots (a, b, c, roots);
  for (unsigned i = 0; i < nroots; i++)
    {
      dist_t root = roots[i];
      if (root > ray.t0 && root < ray.t1)
	{
	  coord_t z = o.z + root * d.z;
	  if (z >= -1 && z <= 1)
	    return root;
	}
    }

  return 0;
}



// If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
// to reflect the point of intersection, and return a Surface::IsecInfo
// object describing the intersection (which should be allocated using
// placement-new with ISEC_CTX); otherwise return zero.
//
const Surface::IsecInfo *
Cylinder::intersect (Ray &ray, const IsecCtx &isec_ctx) const
{
  Ray oray = world_to_local (ray);
  dist_t t = cylinder_intersect (oray);

  if (t != 0)
    {
      ray.t1 = t;

      coord_t ix = oray.origin.x + t * oray.dir.x;
      coord_t iy = oray.origin.y + t * oray.dir.y;

      return new (isec_ctx) IsecInfo (this, ix, iy);
    }
  else
    return 0;
}

// Create an Intersect object for this intersection.
//
Intersect
Cylinder::IsecInfo::make_intersect (const Ray &ray, Trace &trace) const
{
  Pos point = ray.end ();
  Vec norm = cylinder->normal_to_world (Vec (isec_x, isec_y, 0)).unit ();
  Vec t = cylinder->local_to_world (Vec (0, 0, 1)).unit ();
  Vec s = cross (norm, t);
  return Intersect (ray, cylinder, Frame (point, s, t, norm), trace);
}

// Return the strongest type of shadowing effect this surface has on
// RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
// otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
// returned; otherwise, Material::SHADOW_MEDIUM is returned.
//
Material::ShadowType
Cylinder::shadow (const ShadowRay &sray) const
{
  Ray oray = world_to_local (sray.as_ray ());

  if (cylinder_intersect (oray) != 0)
    return material->shadow_type;
  else
    return Material::SHADOW_NONE;
}


// arch-tag: 1a4758de-f640-4ea6-abf2-2626070847e5
