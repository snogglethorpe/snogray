// space.h -- Space-division abstraction (hierarchically arranges 3D space)
//
//  Copyright (C) 2005, 2007, 2008, 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SPACE_H__
#define __SPACE_H__

#include "ray.h"
#include "surface.h"


namespace snogray {


class Space
{
public:

  virtual ~Space () { }

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
  Material::ShadowType shadow (const ShadowRay &ray, Surface::IsecCtx &isec_ctx,
			       const Light *hint_light = 0)
    const;

  // Return true if any object intersects RAY.
  //
  // If HINT_LIGHT is non-zero, then the shadow-hint entry for HINT_LIGHT
  // should be updated to hold the first object which results in an opaque
  // shadow.
  //
  bool shadows (const ShadowRay &ray, Surface::IsecCtx &isec_ctx,
		const Light *hint_light = 0)
    const;

  // A callback for `for_each_possible_intersector'.  Users of
  // `for_each_possible_intersector' must subclass this, providing their
  // own operator() method, and adding any extra data fields they need.
  //
  struct IntersectCallback
  {
    IntersectCallback () : stop (false) { }

    virtual ~IntersectCallback () { }

    // Test SURF to see if it really intersects, and return true if so.
    // Returning true does not necessarily stop the search; to do that,
    // call the IntersectCallback::stop_intersection method.
    //
    virtual bool operator() (const Surface *surf) = 0;

    void stop_iteration () { stop = true; }

    // If set to true, return from iterator immediately
    //
    bool stop;
  };

  // Call CALLBACK for each surface in the voxel tree that _might_
  // intersect RAY (any further intersection testing needs to be done
  // directly on the resulting surfaces).  TRACE is used to access
  // various cache data structures.  ISEC_STATS will be updated.
  //
  virtual void for_each_possible_intersector (const Ray &ray,
					      IntersectCallback &callback,
					      Trace &trace,
					      TraceStats::IsecStats &isec_stats)
    const = 0;


protected:

  // This structure is used to hold state during the search.  It will
  // probably be subclassed by specific types of accelerator.
  //
  struct SearchState
  {
    SearchState (IntersectCallback &_callback)
      : callback (_callback),
	node_intersect_calls (0), surf_isec_tests (0), surf_isec_hits (0)
    { }

    // Update the global statistical counters in ISEC_STATS with the
    // results from this search.
    //
    void update_isec_stats (TraceStats::IsecStats &isec_stats)
    {
      isec_stats.surface_intersects_tests   += surf_isec_tests;
      isec_stats.surface_intersects_hits    += surf_isec_hits;
      isec_stats.space_node_intersect_calls += node_intersect_calls;
    }

    // Call back to do surface testing.
    //
    IntersectCallback &callback;

    // Keep track of some intersection statistics.
    //
    unsigned long node_intersect_calls;
    unsigned surf_isec_tests, surf_isec_hits;
  };
  
};


}

#endif /* __SPACE_H__ */


// arch-tag: b992c2ec-257d-4b88-9001-83a90353e668
