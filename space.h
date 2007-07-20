// space.h -- Space-division abstraction (hierarchically arranges 3D space)
//
//  Copyright (C) 2005, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SPACE_H__
#define __SPACE_H__


#include "ray.h"
#include "surface.h"


namespace snogray {

class BBox;


class Space
{
public:

  virtual ~Space () { }

  // Add SURFACE to the space
  //
  void add (Surface *surface);
  virtual void add (Surface *surface, const BBox &surface_bbox) = 0;

  // Return the closest surface in this space which intersects the
  // bounded-ray RAY, or zero if there is none.  RAY's length is shortened
  // to reflect the point of intersection.
  //
  const Surface::IsecInfo *intersect (Ray &ray, Trace &trace) const;

  // Return some surface shadowing LIGHT_RAY from LIGHT, or 0 if there
  // is no shadowing surface.  If a surface is returned, and it is _not_
  // an "opaque" surface (shadow-type Material::SHADOW_OPAQUE), then it
  // is guaranteed there are no opaque surfaces casting a shadow.
  //
  // ISEC is the intersection for which we are searching for shadow-casters.
  //
  // This is similar, but not identical to the behavior of the
  // `intersect' method -- `intersect' always returns the closest
  // surface and makes no guarantees about the properties of further
  // intersections.
  //
  const Surface *shadow_caster (const Ray &light_ray, const Intersect &isec,
				Trace &trace, const Light *light)
    const;
    
  // Statistics for a space
  //
  struct Stats
  {
    Stats ()
      : num_nodes (0), num_leaf_nodes (0),
	num_surfaces (0), num_dup_surfaces (0),
	max_depth (0), avg_depth (0)
    { }

    unsigned long num_nodes;
    unsigned long num_leaf_nodes;
    unsigned long num_surfaces;
    unsigned long num_dup_surfaces;
    unsigned max_depth;
    float avg_depth;
  };

  // Return various statistics about this space
  //
  virtual Stats stats () const = 0;

  // A callback for `for_each_possible_intersector'.  Users of
  // `for_each_possible_intersector' must subclass this, providing their
  // own operator() method, and adding any extra data fields they need.
  //
  struct IntersectCallback
  {
    IntersectCallback () : stop (false), node_intersect_calls (0) { }

    virtual ~IntersectCallback () { }

    virtual void operator() (Surface *) = 0;

    void stop_iteration () { stop = true; }

    // If set to true, return from iterator immediately
    //
    bool stop;

    // Keep track of some intersection statistics.
    //
    unsigned long node_intersect_calls;
  };

  // Call CALLBACK for each surface in the voxel tree that _might_
  // intersect RAY (any further intersection testing needs to be done
  // directly on the resulting surfaces).
  //
  virtual void for_each_possible_intersector (const Ray &ray,
					      IntersectCallback &callback)
    const = 0;
};


}

#endif /* __SPACE_H__ */


// arch-tag: b992c2ec-257d-4b88-9001-83a90353e668
