// space.h -- Space-division abstraction (hierarchically arranges 3D space)
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SPACE_H__
#define __SPACE_H__

#include "surface.h"

namespace Snogray {

class Space
{
public:

  class IntersectCallback; // fwd decl

  virtual ~Space () { }

  // Add SURFACE to the space
  //
  void add (Surface *surface) { add (surface, surface->bbox ()); }
  virtual void add (Surface *surface, const BBox &surface_bbox) = 0;

  // Call CALLBACK for each surface in the voxel tree that _might_
  // intersect RAY (any further intersection testing needs to be done
  // directly on the resulting surfaces).
  //
  virtual void for_each_possible_intersector (const Ray &ray,
					      IntersectCallback &callback)
    const = 0;
    
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

  // Statistics for runtime intersections
  //
  struct IsecStats
  {
    IsecStats () : node_intersect_calls (0) { }

    unsigned long long node_intersect_calls;
  };

  // A callback for `for_each_possible_intersector'.  Users of
  // `for_each_possible_intersector' must subclass this, providing their
  // own operator() method, and adding any extra data fields they need.
  //
  struct IntersectCallback
  {
    IntersectCallback (IsecStats *_stats)
      : stop (false), stats (_stats)
    { }

    virtual ~IntersectCallback () { }

    virtual void operator() (Surface *) = 0;

    void stop_iteration () { stop = true; }

    // If set to true, return from iterator immediately
    bool stop;

    // This is used for stats gathering
    IsecStats *stats;
  };

  // Return various statistics about this space
  //
  virtual Stats stats () const = 0;
};

}

#endif /* __SPACE_H__ */

// arch-tag: b992c2ec-257d-4b88-9001-83a90353e668
