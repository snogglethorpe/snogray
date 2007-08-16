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
  const Surface::IsecInfo *intersect (Ray &ray,
				      const Surface::IsecCtx &isec_ctx)
    const;

  // Return the strongest type of shadowing effect any object in this space
  // has on RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
  // otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
  // returned; otherwise, Material::SHADOW_MEDIUM is returned.
  //
  // If HINT_LIGHT is non-zero, then the shadow-hint entry for HINT_LIGHT
  // should be updated to hold the first object which results in an opaque
  // shadow.
  //
  Material::ShadowType shadow (const ShadowRay &ray, Trace &trace,
			       const Light *hint_light = 0)
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
