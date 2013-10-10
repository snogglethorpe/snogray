// octree.cc -- Voxel tree datatype (hierarchically arranges 3D space)
//
//  Copyright (C) 2005-2007, 2009, 2010, 2012, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "util/grab.h"
#include "geometry/bbox.h"
#include "isec-cache.h"

#include "octree.h"
#include "octree-node.h"


using namespace snogray;



// Octree::SearchState

struct Octree::SearchState : Space::SearchState
{
  SearchState (const Octree &_octree, const Ray &_ray,
	       IntersectCallback &_callback, IsecCache &_negative_isec_cache)
    : Space::SearchState (_callback),
      ray (_ray),
      ray_origin_octant ((ray.dir.x >= 0 ? Node::X_LO : Node::X_HI)
			 | (ray.dir.y >= 0 ? Node::Y_LO : Node::Y_HI)
			 | (ray.dir.z >= 0 ? Node::Z_LO : Node::Z_HI)),
      nodes (_octree.nodes), surface_ptrs (_octree.surface_ptrs),
      negative_isec_cache (_negative_isec_cache),
      neg_cache_hits (0), neg_cache_collisions (0)
  { }


  // Call our callback for each surface that intersects our ray in the
  // octree underneath node NODE_INDEX.  The remaining parameters are
  // pre-computed intersection points in the ray's parametric space, of
  // the ray in the various planes bounding that node's volume.
  //
  void for_each_possible_intersector (unsigned node_index,
				      dist_t x_min_t, dist_t x_max_t,
				      dist_t y_min_t, dist_t y_max_t,
				      dist_t z_min_t, dist_t z_max_t);

  // Update the global statistical counters in ISEC_STATS with the
  // results from this search.
  //
  void update_isec_stats (RenderStats::IsecStats &isec_stats)
  {
    isec_stats.neg_cache_collisions += neg_cache_collisions;
    isec_stats.neg_cache_hits	      += neg_cache_hits;

    Space::SearchState::update_isec_stats (isec_stats);
  }

  // Ray being searched along.  Note that this must be a reference,
  // not a copy, as the ray it points to may actually change (and
  // ignoring those changes would mean we lose the opportunity to
  // prune the search).
  //
  const Ray &ray;

  // A bitmask of bits from Octree::Node::DirBits giving the quadrant
  // which RAY start in: for each <axis> in x, y, z, if RAY's
  // direction is negative in that axis, then use <axis>_HI, otherwise
  // use <axis>_LO.
  //
  unsigned ray_origin_octant;

  // Node and surface-pointer vectors from Octree.
  //
  const std::vector<Node> &nodes;
  const std::vector<const Surface::Renderable *> &surface_ptrs;

  // Cache of negative surface intersection test results, so we can
  // avoid testing the same object twice.
  //
  IsecCache &negative_isec_cache;

  // Keep track of some statics for the negative intersection cache.
  //
  unsigned neg_cache_hits, neg_cache_collisions;
};



// Ray intersection testing (Octree::for_each_possible_intersector)

// Call CALLBACK for each surface in the voxel tree that _might_
// intersect RAY (any further intersection testing needs to be done
// directly on the resulting surfaces).  CONTEXT is used to access
// various cache data structures.  ISEC_STATS will be updated.
//
void
Octree::for_each_possible_intersector (const Ray &ray,
				       IntersectCallback &callback,
				       RenderContext &context,
				       RenderStats::IsecStats &isec_stats)
  const
{
  if (! nodes.empty ())
    {
      //
      // Compute the intersections of RAY with each of ROOT's bounding
      // planes in RAY's parametric space.  Because ROOT's volume is
      // aligned with the coordinate axes, this is very simple.
      //
      // Note that we ignore the extent of RAY during these
      // calculations, and treat RAY as an infinite line.
      //

      dist_t inv_x = ray.dir.x == 0 ? dist_t (1e9) : 1 / ray.dir.x;
      dist_t x_min_t = (origin.x - ray.origin.x) * inv_x;
      dist_t x_max_t = (origin.x - ray.origin.x + size) * inv_x;
      if (x_min_t > x_max_t)
	std::swap (x_min_t, x_max_t);

      dist_t inv_y = ray.dir.y == 0 ? dist_t (1e9) : 1 / ray.dir.y;
      dist_t y_min_t = (origin.y - ray.origin.y) * inv_y;
      dist_t y_max_t = (origin.y - ray.origin.y + size) * inv_y;
      if (y_min_t > y_max_t)
	std::swap (y_min_t, y_max_t);

      dist_t inv_z = ray.dir.z == 0 ? dist_t (1e9) : 1 / ray.dir.z;
      dist_t z_min_t = (origin.z - ray.origin.z) * inv_z;
      dist_t z_max_t = (origin.z - ray.origin.z + size) * inv_z;
      if (z_min_t > z_max_t)
	std::swap (z_min_t, z_max_t);

      //
      // Note that we don't check to see if RAY intersects ROOT, as
      // this is done immediately when we start the actual search.  By
      // doing a bounds-check here and aborting early if RAY doesn't
      // intersect the space at all (this generally seems to be true
      // for about 1% of rays), we could save some time doing setup,
      // but it turns out that it makes essentially no difference in
      // runtime, so we don't bother.
      //

      // Get an IsecCache object.
      //
      Grab<IsecCache> isec_cache_grab (context.isec_cache_pool);

      SearchState ss (*this, ray, callback, *isec_cache_grab);

      // Search starting form the top-level node.
      //
      ss.for_each_possible_intersector (0,
					x_min_t, x_max_t,
					y_min_t, y_max_t,
					z_min_t, z_max_t);

      ss.update_isec_stats (isec_stats);
    }
}



// Ray intersection testing (Octree::SearchState::for_each_possible_intersector)

// Call our callback for each surface that intersects our ray in the
// octree underneath node NODE_INDEX.  The remaining parameters are
// pre-computed intersection points in the ray's parametric space, of
// the ray in the various planes bounding that node's volume.
//
// This method is critical for speed.
//
void
Octree::SearchState::for_each_possible_intersector (
		       unsigned node_index,
		       dist_t x_min_t, dist_t x_max_t,
		       dist_t y_min_t, dist_t y_max_t,
		       dist_t z_min_t, dist_t z_max_t)
{
  node_intersect_calls++;

  // Return immediately if the ray doesn't intersect this node.
  //
  dist_t min_t = max (ray.t0, max (x_min_t, max (y_min_t, z_min_t)));
  dist_t max_t = min (ray.t1, min (x_max_t, min (y_max_t, z_max_t)));
  if (min_t >= max_t)
    return;

  const Node &node = nodes[node_index];

  // Invoke the callback on each of this node's surfaces
  //
  if (node.surface_ptrs_head_index)
    for (unsigned surf_ptr_index = node.surface_ptrs_head_index;
	 surface_ptrs[surf_ptr_index]; surf_ptr_index++)
      {
	const Surface::Renderable *surf = surface_ptrs[surf_ptr_index];

	if (! negative_isec_cache.contains (surf))
	  {
	    surf_isec_tests++;

	    if (callback (surf))
	      surf_isec_hits++;
	    else
	      {
		bool collision = negative_isec_cache.add (surf);
		if (collision)
		  neg_cache_collisions++;
	      }
	  }
	else
	  neg_cache_hits++;

	if (callback.stop)
	  return;
      }

  // Recursively deal with any non-null sub-nodes
  //
  if (! node.is_leaf_node ())
    {
      // Calculate half-size of each plane in parametric space.
      //
      dist_t x_half_t = (x_max_t - x_min_t) / 2;
      dist_t y_half_t = (y_max_t - y_min_t) / 2;
      dist_t z_half_t = (z_max_t - z_min_t) / 2;

      // ... and the mid-points between min- and max- parametric values.
      //
      dist_t x_mid_t = x_min_t + x_half_t;
      dist_t y_mid_t = y_min_t + y_half_t;
      dist_t z_mid_t = z_min_t + z_half_t;

      for (unsigned i = 0; i < 8; i++)
	{
	  // CHILD is the child index in "parametric order":  that
	  // is where each bit in CHILD, being "HI" (1) or "LO"
	  // (0) doesn't correspond to high or low in that
	  // dimension in actual physical coordinates, but rather
	  // from the viewpoint or the ray's direction.  We can
	  // then use RAY_ORIGIN_OCTANT to translate to "real"
	  // physical order.
	  //
	  // CHILD is mostly the same as I, but we slightly
	  // perturb the bits so that the number of zero-bits
	  // never decreases.  This ensures that we search more
	  // likely child nodes first.
	  //
	  unsigned child = (i == 3) ? 4 : (i == 4) ? 3 : i;

	  // REAL_CHILD is the actual index in
	  // Node::child_node_indices, corresponding to physical
	  // space.
	  //
	  unsigned real_child = child ^ ray_origin_octant;

	  // The index in Octree::nodes of the child.
	  //
	  unsigned child_node_index = node.child_node_indices[real_child];

	  // Test whether there actually is a child node, and if
	  // so whether the ray falls within it; if so, recurse
	  // into that child node.
	  //
	  dist_t t0 = ray.t0, t1 = ray.t1;
	  if (child_node_index
	      && ((child & Node::X_HI) ? (t1 > x_mid_t) : (t0 < x_mid_t))
	      && ((child & Node::Y_HI) ? (t1 > y_mid_t) : (t0 < y_mid_t))
	      && ((child & Node::Z_HI) ? (t1 > z_mid_t) : (t0 < z_mid_t)))
	    {
	      // The lower bounds of the child node in parametric space.
	      //
	      dist_t child_x_min_t
		= x_min_t + ((child & Node::X_HI) ? x_half_t : 0);
	      dist_t child_y_min_t
		= y_min_t + ((child & Node::Y_HI) ? y_half_t : 0);
	      dist_t child_z_min_t
		= z_min_t + ((child & Node::Z_HI) ? z_half_t : 0);

	      // Recurse into the child node.
	      //
	      for_each_possible_intersector (child_node_index,
					     child_x_min_t,
					     child_x_min_t + x_half_t,
					     child_y_min_t,
					     child_y_min_t + y_half_t,
					     child_z_min_t,
					     child_z_min_t + z_half_t);

	      // If iteration was explicitly stopped, return immediately.
	      //
	      if (unlikely (callback.stop))
		return;
	    }
	}
    }
}



// Statistics gathering

// Return various statistics about this octree.
//
Octree::Stats
Octree::stats () const
{
  Stats stats;
  if (! nodes.empty ())
    upd_stats (nodes[0], stats);
  stats.num_dup_surfaces = stats.num_surfaces - num_real_surfaces;
  return stats;
}

// Update STATS to reflect NODE.
//
void
Octree::upd_stats (const Node &node, Stats &stats) const
{
  unsigned num_subnodes = 0;

  // Some fields in STATS are only visible between siblings.  For these, we
  // save the value we get (which reflects our previous siblilngs), and
  // temporarily initialize the field in STATS to be zero for our siblings.
  // We'll then combine the two values at the end of this function.
  //
  unsigned sibling_max_depth = stats.max_depth;
  stats.max_depth = 0;		// will be restored at end of fun
  float sibling_avg_depth = stats.avg_depth;
  stats.avg_depth = 0;		// will be restored at end of fun

  // Get sibling values

  if (! node.is_leaf_node ())
    for (unsigned i = 0; i < 8; i++)
      if (node.child_node_indices[i])
	num_subnodes++, upd_stats (nodes[node.child_node_indices[i]], stats);

  // Now update STATS

  // Num nodes
  //
  stats.num_nodes++;
  if (num_subnodes == 0)
    stats.num_leaf_nodes++;

  // Num surfaces
  //
  for (unsigned spi = node.surface_ptrs_head_index; surface_ptrs[spi]; spi++)
    stats.num_surfaces++;

  // Update `max_depth' field.
  //
  if (stats.max_depth + 1 > sibling_max_depth)
    stats.max_depth++;
  else
    stats.max_depth = sibling_max_depth;

  // Update `avg_depth' field.
  //
  if (num_subnodes != 0)
    stats.avg_depth /= num_subnodes;
  stats.avg_depth += 1 + sibling_avg_depth;
}


// arch-tag: ec7b70cc-3cf6-40f3-9ec6-0ce71dbd20c5
