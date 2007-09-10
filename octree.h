// octree.h -- Voxel tree datatype (hierarchically arranges 3D space)
//
//  Copyright (C) 2005, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __OCTREE_H__
#define __OCTREE_H__

#include <list>

#include "pos.h"
#include "space.h"

namespace snogray {

class Octree : public Space
{
public:

  Octree () : root (0), num_real_surfaces (0) { }
  ~Octree () { delete root; }

  // Add SURFACE to the octree
  //
  virtual void add (const Surface *surface, const BBox &surface_bbox);

  // Call CALLBACK for each surface in the voxel tree that _might_
  // intersect RAY (any further intersection testing needs to be done
  // directly on the resulting surfaces).  TRACE is used to access
  // various cache data structures.  ISEC_STATS will be updated.
  //
  virtual void for_each_possible_intersector (const Ray &ray,
					      IntersectCallback &callback,
					      Trace &trace,
					      TraceStats::IsecStats &isec_stats)
    const;

  // Return various statistics about this octree
  //
  virtual Stats stats () const;

  // One corner of the octree
  //
  Pos origin;

  // The size of the octree (in all dimensions)
  //
  dist_t size;


private:  

  struct SearchState : Space::SearchState
  {
    SearchState (IntersectCallback &_callback, IsecCache &_negative_isec_cache)
      : Space::SearchState (_callback),
	negative_isec_cache (_negative_isec_cache),
	neg_cache_hits (0), neg_cache_collisions (0)
    { }

    // Update the global statistical counters in ISEC_STATS with the
    // results from this search.
    //
    void update_isec_stats (TraceStats::IsecStats &isec_stats)
    {
      isec_stats.neg_cache_collisions += neg_cache_collisions;
      isec_stats.neg_cache_hits	      += neg_cache_hits;

      Space::SearchState::update_isec_stats (isec_stats);
    }
    
    // Cache of negative surface intersection test results, so we can
    // avoid testing the same object twice.
    //
    IsecCache &negative_isec_cache;

    // Keep track of some statics for the negative intersection cache.
    //
    unsigned neg_cache_hits, neg_cache_collisions;
  };

  // The current root of this octree is too small to encompass SURFACE;
  // add surrounding levels of nodes until one can hold SURFACE, and make that
  // the new root node.
  //
  void grow_to_include (const Surface *surface, const BBox &surface_bbox);

  // A octree node is one level of the tree, containing a cubic volume
  // (the size is not explicitly stored in the node).  It is divided
  // into 8 equally-sized sub-nodes by splitting the node equally along
  // each axis.
  //
  struct Node
  {
    Node ()
      : x_lo_y_lo_z_lo (0), x_lo_y_lo_z_hi (0),
	x_lo_y_hi_z_lo (0), x_lo_y_hi_z_hi (0),
	x_hi_y_lo_z_lo (0), x_hi_y_lo_z_hi (0),
	x_hi_y_hi_z_lo (0), x_hi_y_hi_z_hi (0),
	has_subnodes (false)
    { }
    ~Node ();

    // Version of `for_each_possible_intersector' used for recursive
    // voxel tree searching.  The additional parameters are pre-computed
    // intersection points of the ray being intersected in the various
    // planes bounding this node's volume (we don't actually need the
    // ray itself).
    //
    void for_each_possible_intersector (const Ray &ray,
					SearchState &ss,
					const Pos &x_min_isec,
					const Pos &x_max_isec,
					const Pos &y_min_isec,
					const Pos &y_max_isec,
					const Pos &z_min_isec,
					const Pos &z_max_isec)
      const;

    // Add SURFACE, with bounding box SURFACE_BBOX, to this node or some subnode;
    // SURFACE is assumed to fit.  X, Y, Z, and SIZE indicate the volume this
    // node encompasses.
    //
    void add (const Surface *surface, const BBox &surface_bbox,
	      coord_t x, coord_t y, coord_t z, dist_t size);

    // A helper method that calls NODE's `add' method, after first
    // making sure that NODE exists (creating it if it does not).
    //
    void add_or_create (Node* &node,
			const Surface *surface, const BBox &surface_bbox,
			coord_t x, coord_t y, coord_t z, dist_t size)
    {
      if (! node)
	{
	  node = new Node;
	  has_subnodes = true;
	}

      node->add (surface, surface_bbox, x, y, z, size);
    }

    // Update STATS to reflect this node.
    //
    void upd_stats (Stats &stats) const;

    // Surfaces at this level of the tree.  All surfaces listed in a node
    // must fit entirely within it.  Any given surface is only present in
    // a single node.
    //
    std::list<const Surface *> surfaces;

    // The sub-nodes of this node; each sub-node is exactly half the
    // size of this node in all dimensions, so in total there are eight.
    //
    Node *x_lo_y_lo_z_lo, *x_lo_y_lo_z_hi, *x_lo_y_hi_z_lo, *x_lo_y_hi_z_hi;
    Node *x_hi_y_lo_z_lo, *x_hi_y_lo_z_hi, *x_hi_y_hi_z_lo, *x_hi_y_hi_z_hi;

    // True if any of the above subnodes is non-null.
    //
    bool has_subnodes;
  };

  // The root of the tree
  //
  Node *root;

  // The number of "real" surfaces added to the octree.
  //
  unsigned long num_real_surfaces;
};

}

#endif /* __OCTREE_H__ */

// arch-tag: 0b44a400-1a03-4967-ac84-a8984a4f2752
