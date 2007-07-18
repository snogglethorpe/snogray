// surface.h -- Physical surface
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SURFACE_H__
#define __SURFACE_H__

#include "vec.h"
#include "color.h"
#include "ray.h"
#include "bbox.h"
#include "intersect.h"
#include "material.h"

namespace snogray {

class Material;
class Space;

// This class is used to record the "parameters" of a ray-surface
// intersection.  In particular for triangles, u and v are the barycentric
// coordinates of the intersection.
//
struct IsecParams
{
  dist_t u, v;
};

// A surface is the basic object scenes are constructed of.
// Surfaces exist in 3D space, but are basically 2D -- volumetric
// properties are only modelled in certain special cases.
//
class Surface 
{
public:

  Surface (const Material *mat) : material (mat) { }
  virtual ~Surface () { }

  // If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
  // to reflect the point of intersection, and return true; otherwise
  // return false.  ISEC_PARAMS maybe used to pass information to a later
  // call to Surface::intersect_info.
  //
  virtual bool intersect (Ray &ray, IsecParams &isec_params) const;

  // Return an Intersect object containing details of the intersection of
  // RAY with this surface; it is assumed that RAY does actually hit the
  // surface, and RAY's maximum bound (Ray::t1) gives the exact point of
  // intersection (the `intersect' method modifies RAY so that this is
  // true).  ISEC_PARAMS contains other surface-specific parameters
  // calculated by the previous call to Surface::intersects method.
  //
  virtual Intersect intersect_info (const Ray &ray,
				    const IsecParams &isec_params,
				    Trace &trace)
    const;

  // Like `intersects', but rejects an intersection if this object is in
  // the smoothing-group SMOOTH_GROUP, and RAY is hitting the back-face of
  // the object; FROM_REVERSE inverts the sense of the smoothing-group face test
  // (if true, shadows from the _front_ of an object in the same
  // smoothing-group are rejected).
  //
  bool shadows (const Ray &ray, const Intersect &isec) const
  {
    IsecParams isec_params;	// not used
    Ray temp_ray (ray);

    if (intersect (temp_ray, isec_params))
      {
	if (isec.smoothing_group)
	  return confirm_shadow (ray, temp_ray.t1, isec);

	return true;
      }

    return false;
  }

  // Confirm that this surfaces blocks RAY, which emanates from the
  // intersection ISEC.  DIST is the distance between ISEC and the position
  // where RAY intersects this surface.
  //
  virtual bool confirm_shadow (const Ray &ray, dist_t dist,
			       const Intersect &isec)
  const;

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const;

  // Add this (or some other ...) surfaces to SPACE
  //
  virtual void add_to_space (Space &space);
 
  // The "smoothing group" this surface belongs to, or zero if it belongs
  // to none.  The smoothing group affects shadow-casting: if two objects
  // are in the same smoothing group, they will not be shadowed by
  // back-surface shadows from each other; typically all triangles in a
  // mesh are in the same smoothing group.
  //
  virtual const void *smoothing_group () const;

  const Material *material;
};

}

#endif /* __SURFACE_H__ */

// arch-tag: 85997b65-c9ab-4542-80be-0c3a114593ba
