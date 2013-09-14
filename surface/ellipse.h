// ellipse.h -- Ellipse surface
//
//  Copyright (C) 2007-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_ELLIPSE_H
#define SNOGRAY_ELLIPSE_H

#include "geometry/tripar-isec.h"

#include "primitive.h"


namespace snogray {


// A ellipse object.
//
// It is implemented as a parallelogram surface with additional
// intersection constraints.
//
class Ellipse : public Primitive
{
public:

  // Create an ellipse with a center at CENTER, and orthogonal radius
  // vectors RADIUS1 and RADIUS2.  The ellipse will lie in the plane
  // defined by the points CENTER, CENTER+RADIUS1, and CENTER+RADIUS2.
  // If RADIUS1 and RADIUS2 are not orthogonal, then the resulting
  // ellipse will be skewed.
  //
  Ellipse (const Ref<const Material> &mat,
	   const Pos &center, const Vec &radius1, const Vec &radius2)
    : Primitive (mat),
      corner (center - radius1 - radius2),
      edge1 (radius1 * 2), edge2 (radius2 * 2),
      normal (cross (edge2, edge1).unit ())
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

  // Return true if this surface intersects RAY; if true is returned, the
  // intersection parameters are return in T, U, and V.
  //
  bool intersects (const Ray &ray, dist_t &t, dist_t &u, dist_t &v) const
  {
    if (parallelogram_intersects (corner, edge1, edge2, ray, t, u, v))
      {
	// X and Y are the coordinates of intersection point relative to
	// the ellipse center in the plane of the ellipse, each scaled
	// according to the length of the corresponding radius / 2.
	//
	dist_t x = u - dist_t (0.5), y = v - dist_t (0.5);

	return x*x + y*y <= dist_t (0.25); // 0.25 == 0.5^2
      }

    return false;
  }

  // Return true if this surface intersects a ray from RAY_ORIGIN in
  // direction RAY_DIR.  If true is returned, the intersection
  // parameters are return in T, U, and V.
  //
  bool intersects (const Pos &ray_origin, const Vec &ray_dir,
		   dist_t &t, dist_t &u, dist_t &v)
    const
  {
    if (parallelogram_intersects (corner, edge1, edge2, ray_origin, ray_dir,
				  t, u, v))
      {
	// X and Y are the coordinates of intersection point relative to
	// the ellipse center in the plane of the ellipse, each scaled
	// according to the length of the corresponding radius / 2.
	//
	dist_t x = u - dist_t (0.5), y = v - dist_t (0.5);

	return x*x + y*y <= dist_t (0.25); // 0.25 == 0.5^2
      }

    return false;
  }

  // A parallelogram which surrounds this ellipse.
  //
  Pos corner;
  Vec edge1, edge2;

  // Normal vector
  //
  Vec normal;
};


}

#endif /* SNOGRAY_ELLIPSE_H */
