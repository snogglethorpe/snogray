// obj.h -- Root of object class hierarchy
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __OBJ_H__
#define __OBJ_H__

#include "vec.h"
#include "color.h"
#include "ray.h"
#include "bbox.h"

namespace Snogray {

class Material;
class Voxtree;

class Obj 
{
public:

  Obj () : no_shadow (false) { }
  virtual ~Obj (); // stop gcc bitching

  // Return the distance from RAY's origin to the closest intersection
  // of this object with RAY, or 0 if there is none.  RAY is considered
  // to be unbounded.
  //
  virtual dist_t intersection_distance (const Ray &ray) const;

  // Given that RAY's origin is known to lie on this object, return the
  // distance from RAY's origin to the _next_ closest intersection of
  // this object with RAY, or 0 if there is none.  For non-convex
  // objects such as triangles, the default implementation which always
  // returns 0 is correct.  RAY is considered to be unbounded.
  //
  virtual dist_t next_intersection_distance (const Ray &ray) const;

  // If this object intersects the bounded-ray RAY, change RAY's length
  // to reflect the point of intersection, and return true; otherwise
  // return false.  If ORIGIN points to this object (meaning it is the
  // origin of RAY), the first intersection is ignored, and only a 2nd,
  // farther, intersection (if any; see `next_intersection_distance') is
  // considered.
  //
  bool intersect (Ray &ray, const Obj *origin) const
  {
    dist_t dist
      = (origin == this
	 ? next_intersection_distance (ray)
	 : intersection_distance (ray));

    if (dist > 0 && dist < ray.len)
      {
	ray.set_len (dist);
	return true;
      }
    else
      return false;
  }

  // A simpler interface to intersection: just returns true if this object
  // intersects the bounded-ray RAY.  Unlike the `intersect' method, RAY is
  // never modified.
  //
  bool intersects (const Ray &ray) const
  {
    dist_t dist = intersection_distance (ray);
    return dist > 0 && dist < ray.len;
  }

  // Returns the normal vector for this surface at POINT.
  // INCOMING is the direction of the incoming ray that has hit POINT;
  // this can be used by dual-sided objects to decide which side's
  // normal to return.
  //
  virtual Vec normal (const Pos &point, const Vec &incoming) const;

  // Return a bounding box for this object.
  //
  virtual BBox bbox () const;

  // Returns the material this object is made from
  //
  virtual const Material *material () const;

  // Add this (or some other ...) objects to SPACE
  //
  virtual void add_to_space (Voxtree &space);

  // Ideally this would be represented via some virtual method instead, but
  // it gets used _very_ often and doing so slows down tracing by about 2%.
  //
  bool no_shadow;
};

}

#endif /* __OBJ_H__ */

// arch-tag: 85997b65-c9ab-4542-80be-0c3a114593ba
