// tripar.h -- Triangle/parallelogram surface
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TRIPAR_H__
#define __TRIPAR_H__

#include "tripar-isec.h"

#include "surface.h"


namespace snogray {


class Tripar : public Surface
{
public:

  Tripar (const Ref<const Material> &mat,
	  const Pos &_v0, const Vec &_e1, const Vec &_e2,
	  bool _parallelogram = false)
    : Surface (mat), v0 (_v0), e1 (_e1), e2 (_e2),
      parallelogram (_parallelogram)
  { }

  // If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
  // to reflect the point of intersection, and return a Surface::IsecInfo
  // object describing the intersection (which should be allocated using
  // placement-new with ISEC_CTX); otherwise return zero.
  //
  virtual const IsecInfo *intersect (Ray &ray, const IsecCtx &isec_ctx) const;

  // Return the strongest type of shadowing effect this surface has on
  // RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
  // otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
  // returned; otherwise, Material::SHADOW_MEDIUM is returned.
  //
  virtual Material::ShadowType shadow (const ShadowRay &ray,
				       const IsecCtx &isec_ctx)
    const;

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const;

private:

  struct IsecInfo : public Surface::IsecInfo
  {
    IsecInfo (const Ray &ray, const Tripar *_tripar, dist_t _u, dist_t _v)
      : Surface::IsecInfo (ray), tripar (_tripar), u (_u), v (_v)
    { }
    virtual Intersect make_intersect (Trace &trace) const;
    virtual const Surface *surface () const { return tripar; }
    const Tripar *tripar;
    dist_t u, v;
  };

  // Return true if this surface intersects RAY; if true is returned, the
  // intersection parameters are return in T, U, and V.
  //
  bool intersects (const Ray &ray, dist_t &t, dist_t &u, dist_t &v) const
  {
    return tripar_intersect (v0, e1, e2, parallelogram, ray, t, u, v);
  }

  Pos v0;
  Vec e1, e2;

  bool parallelogram;
};


}

#endif // __TRIPAR_H__


// arch-tag: cf7f49db-937d-4328-b7c0-3959241a191e
