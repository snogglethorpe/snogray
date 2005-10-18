// surface.h -- Physical surface
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
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
#include "material.h"

namespace Snogray {

class Material;
class Voxtree;

// A surface is the basic object scenes are constructed of.
// Surfaces exist in 3D space, but are basically 2D -- volumetric
// properties are only modelled in certain special cases.
//
class Surface 
{
public:

  Surface (Material::ShadowType _shadow_type = Material::SHADOW_OPAQUE)
    : shadow_type (_shadow_type)
  { }
  virtual ~Surface (); // stop gcc bitching

  // If this surface intersects the bounded-ray RAY, change RAY's length to
  // reflect the point of intersection, and return true; otherwise return
  // false.
  //
  // NUM is which intersection to return, for non-flat surfaces that may
  // have multiple intersections -- 0 for the first, 1 for the 2nd, etc
  // (flat surfaces will return failure for anything except 0).
  //
  bool intersect (Ray &ray, unsigned num = 0) const
  {
    dist_t dist = intersection_distance (ray, num);

    if (dist > 0 && dist < ray.len)
      {
	ray.set_len (dist);
	return true;
      }
    else
      return false;
  }

  // A simpler interface to intersection: just returns true if this surface
  // intersects the bounded-ray RAY.  Unlike the `intersect' method, RAY is
  // never modified.
  //
  bool intersects (const Ray &ray, unsigned num = 0) const
  {
    dist_t dist = intersection_distance (ray, num);
    return dist > 0 && dist < ray.len;
  }

  // Return the distance from RAY's origin to the closest intersection
  // of this surface with RAY, or 0 if there is none.  RAY is considered
  // to be unbounded.
  //
  // NUM is which intersection to return, for non-flat surfaces that may
  // have multiple intersections -- 0 for the first, 1 for the 2nd, etc
  // (flat surfaces will return failure for anything except 0).
  //
  virtual dist_t intersection_distance (const Ray &ray, unsigned num = 0) const;

  // Returns the normal vector for this surface at POINT.
  // INCOMING is the direction of the incoming ray that has hit POINT;
  // this can be used by dual-sided surfaces to decide which side's
  // normal to return.
  //
  virtual Vec normal (const Pos &point, const Vec &incoming) const;

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const;

  // Returns the material this surface is made from
  //
  virtual const Material *material () const;

  // Add this (or some other ...) surfaces to SPACE
  //
  virtual void add_to_space (Voxtree &space);

  // What special handling this surface needs when it casts a shadow.
  // This is initialized by calling the surface's material's
  // `shadow_type' method -- it's too expensive to call that during
  // tracing.
  //
  Material::ShadowType shadow_type;

};

}

#endif /* __SURFACE_H__ */

// arch-tag: 85997b65-c9ab-4542-80be-0c3a114593ba
