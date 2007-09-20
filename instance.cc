// instance.cc -- Transformed object subspace
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//
#include<iostream>
#include "intersect.h"
#include "space.h"
#include "octree.h"
#include "shadow-ray.h"
#include "subspace.h"

#include "instance.h"


using namespace snogray;


// If this surface intersects RAY, change RAY's maximum bound (Ray::t1) to
// reflect the point of intersection, and return a Surface::IsecInfo object
// describing the intersection (which should be allocated using
// placement-new with ISEC_CTX); otherwise return zero.
//
Surface::IsecInfo *
Instance::intersect (Ray &ray, const IsecCtx &isec_ctx) const
{
  // Transform the ray for searching our subspace.
  //
  Ray xformed_ray = world_to_local (ray);

  const Surface::IsecInfo *subspace_isec_info
    = subspace->intersect (xformed_ray, isec_ctx);

  if (subspace_isec_info)
    {
      ray.t1 = xformed_ray.t1;
      return new (isec_ctx) IsecInfo (this, xformed_ray, subspace_isec_info);
    }
  else
    return 0;
}

// Create an Intersect object for this intersection.
//
Intersect
Instance::IsecInfo::make_intersect (const Ray &ray, Trace &trace) const
{
  // First make an intersection in our subspace.
  //
  Intersect isec = subspace_isec_info->make_intersect (xformed_ray, trace);

  // Now transform parts of it to be in the global space.
  //
  isec.ray = ray;
  isec.pos = instance->local_to_world (isec.pos);
  isec.n   = instance->normal_to_world (isec.n).unit ();
  isec.s   = instance->normal_to_world (isec.s).unit ();
  isec.t   = instance->normal_to_world (isec.t).unit ();
  isec.v   = -ray.dir.unit ();

  // Self-shadowing is detected via object identity, and object identity is
  // a murky concept for anything in an instance.
  //
  isec.no_self_shadowing = false;

  return isec;
}

// Return the strongest type of shadowing effect this surface has on
// RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
// otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
// returned; otherwise, Material::SHADOW_MEDIUM is returned.
//
Material::ShadowType
Instance::shadow (const ShadowRay &sray) const
{
  // Transform the ray for searching our subspace.
  //
  ShadowRay xformed_sray = world_to_local (sray);
  return subspace->shadow (xformed_sray);
}

// Return a bounding box for this surface.
//
BBox
Instance::bbox () const
{
  return local_to_world (subspace->bbox ());
}


// arch-tag: 8b4091cf-bd1e-4355-a880-3919f8e5b1d0
