// tripar.h -- Triangle/parallelogram surface
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
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

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const;

  // Add a single area light, using this surface's shape, to LIGHTS,
  // with with intensity INTENSITY.  An error will be signaled if this
  // surface does not support lighting.
  //
  virtual void add_light (const TexVal<Color> &intensity,
			  std::vector<Light *> &lights)
    const;

private:

  struct IsecInfo : public Surface::IsecInfo
  {
    IsecInfo (const Ray &ray, const Tripar *_tripar, dist_t _u, dist_t _v)
      : Surface::IsecInfo (ray), tripar (_tripar), u (_u), v (_v)
    { }
    virtual Intersect make_intersect (const Media &media, RenderContext &context)
      const;
    const Tripar *tripar;
    dist_t u, v;
  };

  // Return true if this surface intersects RAY; if true is returned,
  // the intersection parameters are return in T, U, and V.
  //
  bool intersects (const Ray &ray, dist_t &t, dist_t &u, dist_t &v) const
  {
    return tripar_intersects (v0, e1, e2, parallelogram, ray, t, u, v);
  }

  // Return true if this surface intersects a ray from RAY_ORIGIN in
  // direction RAY_DIR.  If true is returned, the intersection
  // parameters are return in T, U, and V.
  //
  bool intersects (const Pos &ray_origin, const Vec &ray_dir,
		   dist_t &t, dist_t &u, dist_t &v)
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

#endif // __TRIPAR_H__


// arch-tag: cf7f49db-937d-4328-b7c0-3959241a191e
