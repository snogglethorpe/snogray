// tripar.h -- Triangle/parallelogram surface
//
//  Copyright (C) 2005-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_TRIPAR_H
#define SNOGRAY_TRIPAR_H

#include "geometry/tripar-isec.h"

#include "primitive.h"


namespace snogray {


class Tripar : public Primitive
{
public:

  Tripar (const Ref<const Material> &mat,
	  const Pos &_v0, const Vec &_e1, const Vec &_e2,
	  bool _parallelogram = false)
    : Primitive (mat), v0 (_v0), e1 (_e1), e2 (_e2),
      parallelogram (_parallelogram)
  { }

  // If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
  // to reflect the point of intersection, and return a Surface::IsecInfo
  // object describing the intersection (which should be allocated using
  // placement-new with CONTEXT); otherwise return zero.
  //
  virtual const IsecInfo *intersect (Ray &ray, RenderContext &context) const;

  // Return true if this surface intersects RAY.
  //
  virtual bool intersects (const Ray &ray, RenderContext &context) const;

  // Return true if this surface completely occludes RAY.  If it does
  // not completely occlude RAY, then return false, and multiply
  // TOTAL_TRANSMITTANCE by the transmittance of the surface in medium
  // MEDIUM.
  //
  // Note that this method does not try to handle non-trivial forms of
  // transparency/translucency (for instance, a "glass" material is
  // probably considered opaque because it changes light direction as
  // well as transmitting it).
  //
  virtual bool occludes (const Ray &ray, const Medium &medium,
			 Color &total_transmittance,
			 RenderContext &context)
    const;

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const;

  // Return a sampler for this surface, or zero if the surface doesn't
  // support sampling.  The caller is responsible for destroying
  // returned samplers.
  //
  virtual Sampler *make_sampler () const;

private:

  class Sampler;
  class IsecInfo;

  // Return true if this surface intersects RAY; if true is returned,
  // the intersection parameters are return in T, U, and V.
  //
  bool intersects (const Ray &ray, dist_t &t, float &u, float &v) const
  {
    return tripar_intersects (v0, e1, e2, parallelogram, ray, t, u, v);
  }

  // Return true if this surface intersects a ray from RAY_ORIGIN in
  // direction RAY_DIR.  If true is returned, the intersection
  // parameters are return in T, U, and V.
  //
  bool intersects (const Pos &ray_origin, const Vec &ray_dir,
		   dist_t &t, float &u, float &v)
    const
  {
    return tripar_intersects (v0, e1, e2, parallelogram, ray_origin, ray_dir,
			      t, u, v);
  }

  Pos v0;
  Vec e1, e2;

  bool parallelogram;
};


}

#endif // SNOGRAY_TRIPAR_H


// arch-tag: cf7f49db-937d-4328-b7c0-3959241a191e
