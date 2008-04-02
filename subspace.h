// subspace.h -- A surface encapsulated into its own subspace
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

#ifndef __SUBSPACE_H__
#define __SUBSPACE_H__

#include <memory>

#include "ref.h"
#include "space.h"
#include "surface.h"
#include "material.h"
#include "shadow-ray.h"


namespace snogray {


// A surface with its own unique "subspace" (acceleration structure).
//
// This is for use with an Instance.
//
class Subspace : public RefCounted
{
public:

  Subspace (Surface *surf) : surface (surf), space (0) { }
  ~Subspace ();

  // If the associated surface intersects RAY, change RAY's maximum bound
  // (Ray::t1) to reflect the point of intersection, and return a
  // Surface::IsecInfo object describing the intersection (which should be
  // allocated using placement-new with ISEC_CTX); otherwise return zero.
  //
  const Surface::IsecInfo *intersect (Ray &ray,
				      const Surface::IsecCtx &isec_ctx)
    const
  {
    ensure_space (isec_ctx.trace.global);
    return space->intersect (ray, isec_ctx);
  }

  // Return the strongest type of shadowing effect the associated surface
  // has on RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
  // otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
  // returned; otherwise, Material::SHADOW_MEDIUM is returned.
  //
  Material::ShadowType shadow (const ShadowRay &sray) const
  {
    Trace &trace = sray.isec.trace;
    ensure_space (trace.global);
    return space->shadow (sray, trace);
  }

  // Return a bounding box for the associated surface.
  //
  BBox bbox () const { return surface->bbox (); }

private:

  // Make sure our acceleration structure is set up.
  //
  void ensure_space (GlobalTraceState &global) const
  {
    if (! space)
      make_space (global);
  }

  // Setup our acceleration structure.
  //
  void make_space (GlobalTraceState &global) const;

  // The top-level surface in this subspace.
  //
  std::auto_ptr<Surface> surface;

  // Space holding everything from SURFACE..
  //
  mutable const Space *space;
};


}


#endif /* __SUBSPACE_H__ */
