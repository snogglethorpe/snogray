// instance.h -- Transformed virtual instance of a surface
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

#ifndef __INSTANCE_H__
#define __INSTANCE_H__

#include "xform.h"
#include "subspace.h"

#include "local-surface.h"


namespace snogray {


class Instance : public LocalSurface
{
public:

  Instance (Subspace *_subspace, const Xform &local_to_world_xform)
    : LocalSurface (0, local_to_world_xform), subspace (_subspace)
  { }

  // If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
  // to reflect the point of intersection, and return a Surface::IsecInfo
  // object describing the intersection (which should be allocated using
  // placement-new with ISEC_CTX); otherwise return zero.
  //
  virtual IsecInfo *intersect (Ray &ray, const IsecCtx &isec_ctx) const;

  // Return the strongest type of shadowing effect this surface has on
  // RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
  // otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
  // returned; otherwise, Material::SHADOW_MEDIUM is returned.
  //
  virtual Material::ShadowType shadow (const ShadowRay &ray) const;

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const;

private:

  struct IsecInfo : public Surface::IsecInfo
  {
    IsecInfo (const Ray &ray,
	      const Instance *_instance,
	      const Surface::IsecInfo *_subspace_isec_info)
      : Surface::IsecInfo (ray),
	instance (_instance),
	subspace_isec_info (_subspace_isec_info)
    { }

    virtual Intersect make_intersect (Trace &trace) const;

    virtual const Surface *surface () const
    {
      return subspace_isec_info->surface();
    }
    virtual const Surface *outermost_surface () const
    {
      return instance;
    }

    const Instance *instance;

    const Surface::IsecInfo *subspace_isec_info;
  };

  // Subspace that we're transforming.
  //
  Subspace *subspace;
};


}


#endif /* __INSTANCE_H__ */

// arch-tag: 553e4e51-689d-4637-8a93-a9ee9db7d8ad
