// subspace.cc -- A surface encapsulated into its own subspace
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <memory>

#include "space.h"
#include "shadow-ray.h"

#include "subspace.h"


using namespace snogray;


// Make sure our acceleration structure is set up.
//
inline void
Subspace::ensure_space (GlobalTraceState &global) const
{
  if (! space.get ())
    {
      std::auto_ptr<SpaceBuilder> space_builder
	(global.space_builder_builder->make_space_builder ());

      surface->add_to_space (*space_builder);

      space.reset (space_builder->make_space ());
    }
}


// If this surface intersects RAY, change RAY's maximum bound (Ray::t1) to
// reflect the point of intersection, and return a Surface::IsecInfo object
// describing the intersection (which should be allocated using
// placement-new with ISEC_CTX); otherwise return zero.
//
const Surface::IsecInfo *
Subspace::intersect (Ray &ray, const IsecCtx &isec_ctx) const
{
  ensure_space (isec_ctx.trace.global);
  return space->intersect (ray, isec_ctx);
}

// Return the strongest type of shadowing effect this surface has on
// RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
// otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
// returned; otherwise, Material::SHADOW_MEDIUM is returned.
//
Material::ShadowType
Subspace::shadow (const ShadowRay &sray) const
{
  Trace &trace = sray.isec.trace;
  ensure_space (trace.global);
  return space->shadow (sray, trace);
}

// Return a bounding box for this surface.
//
BBox
Subspace::bbox () const
{
  return surface->bbox ();
}
