// ellipse.h -- Ellipse surface
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

#ifndef __ELLIPSE_H__
#define __ELLIPSE_H__

#include "tripar-isec.h"

#include "surface.h"


namespace snogray {


// A ellipse object.
//
// It is implemented as a parallelogram surface with additional
// intersection constraints.
//
class Ellipse : public Surface
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
    : Surface (mat),
      corner (center - radius1 - radius2),
      edge1 (radius1 * 2), edge2 (radius2 * 2)
  { }

  // If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
  // to reflect the point of intersection, and return a Surface::IsecInfo
  // object describing the intersection (which should be allocated using
  // placement-new with CONTEXT); otherwise return zero.
  //
  virtual const IsecInfo *intersect (Ray &ray, RenderContext &context) const;

  // Return the strongest type of shadowing effect this surface has on
  // RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
  // otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
  // returned; otherwise, Material::SHADOW_MEDIUM is returned.
  //
  virtual Material::ShadowType shadow (const ShadowRay &ray,
				       RenderContext &context)
    const;

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const;

private:

  struct IsecInfo : public Surface::IsecInfo
  {
    IsecInfo (const Ray &ray, const Ellipse *_ellipse)
      : Surface::IsecInfo (ray), ellipse (_ellipse)
    { }
    virtual Intersect make_intersect (Trace &trace) const;
    virtual const Surface *surface () const { return ellipse; }
    const Ellipse *ellipse;
  };

  // Return true if this surface intersects RAY; if true is returned, the
  // intersection parameters are return in T, U, and V.
  //
  bool intersects (const Ray &ray, dist_t &t, dist_t &u, dist_t &v) const
  {
    if (parallelogram_intersect (corner, edge1, edge2, ray, t, u, v))
      {
	// X and Y are the coordinates of intersection point relative to
	// the ellipse center in the plane of the ellipse, each scaled
	// according to the length of the corresponding radius / 2.
	//
	dist_t x = u - 0.5f, y = v - 0.5f;

	return x*x + y*y <= 0.25f; // 0.25 == 0.5^2
      }

    return false;
  }

  // A parallelogram which surrounds this ellipse.
  //
  Pos corner;
  Vec edge1, edge2;
};


}

#endif /* __ELLIPSE_H__ */
