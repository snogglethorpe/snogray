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

#include "geometry/bbox.h"
#include "util/grab.h"

#include "octree.h"
#include "octree-node.h"


using namespace snogray;



// Octree constructor

// Make a new, empty, octree with the given extent.  This should only
// be invoked directly by Octree::Builder::make_space.
//
Octree::Octree (const Pos &_origin, dist_t _size)
  : origin (_origin), size (_size), num_real_surfaces (0)
{ }



// Octree::SearchState

struct Octree::SearchState : Space::SearchState
{
  SearchState (const Octree &_octree, const Ray &_ray,
	       IntersectCallback &_callback, IsecCache &_negative_isec_cache)
    : Space::SearchState (_callback),
      ray (_ray),
      nodes (_octree.nodes), surface_ptrs (_octree.surface_ptrs),
      negative_isec_cache (_negative_isec_cache),
      neg_cache_hits (0), neg_cache_collisions (0)
  { }


  // Call our callback for each surface that intersects our ray in the
  // octree underneath node NODE_INDEX.  The remaining parameters are
  // pre-computed intersection points of the ray in the various planes
  // bounding that node's volume.
  //
  void for_each_possible_intersector (unsigned node_index,
				      const Pos &x_min_isec,
				      const Pos &x_max_isec,
				      const Pos &y_min_isec,
				      const Pos &y_max_isec,
				      const Pos &z_min_isec,
				      const Pos &z_max_isec);

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

  // Node and surface-pointer vectors from Octree.
  //
  const std::vector<Node> &nodes;
  const std::vector<const Surface *> &surface_ptrs;

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
// directly on the resulting surfaces).  MEDIA is used to access various
// cache data structures.  ISEC_STATS will be updated.
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
      coord_t x_min = origin.x;
      coord_t x_max = origin.x + size;
      coord_t y_min = origin.y;
      coord_t y_max = origin.y + size;
      coord_t z_min = origin.z;
      coord_t z_max = origin.z + size;

      // First make sure RAY is conceivably within the top-most node
      const Pos &rbeg = ray.begin(), &rend = ray.end();
      if ((rbeg.x <= x_max || rend.x <= x_max)
	  && (rbeg.x >= x_min || rend.x >= x_min)
	  && (rbeg.y <= y_max || rend.y <= y_max)
	  && (rbeg.y >= y_min || rend.y >= y_min)
	  && (rbeg.z <= z_max || rend.z <= z_max)
	  && (rbeg.z >= z_min || rend.z >= z_min))
	{
	  // Compute the intersections of RAY with each of ROOT's
	  // bounding planes.  Because ROOT's volume is aligned with the
	  // coordinate axes, this is very simple, if a bit tedious.
	  // Note that we basically ignore the extent of RAY during
	  // these calculations, and treat RAY as an infinite line.

	  dist_t inv_x = ray.dir.x == 0 ? 0 : 1 / ray.dir.x;
	  dist_t inv_y = ray.dir.y == 0 ? 0 : 1 / ray.dir.y;
	  dist_t inv_z = ray.dir.z == 0 ? 0 : 1 / ray.dir.z;

	  dist_t x_min_scale = (x_min - ray.origin.x) * inv_x;
	  const Pos x_min_isec (x_min,
				ray.origin.y + ray.dir.y * x_min_scale,
				ray.origin.z + ray.dir.z * x_min_scale);
	  dist_t x_max_scale = (x_max - ray.origin.x) * inv_x;
	  const Pos x_max_isec (x_max,
				ray.origin.y + ray.dir.y * x_max_scale,
				ray.origin.z + ray.dir.z * x_max_scale);

	  dist_t y_min_scale = (y_min - ray.origin.y) * inv_y;
	  const Pos y_min_isec (ray.origin.x + ray.dir.x * y_min_scale,
				y_min,
				ray.origin.z + ray.dir.z * y_min_scale);
	  dist_t y_max_scale = (y_max - ray.origin.y) * inv_y;
	  const Pos y_max_isec (ray.origin.x + ray.dir.x * y_max_scale,
				y_max,
				ray.origin.z + ray.dir.z * y_max_scale);

	  dist_t z_min_scale = (z_min - ray.origin.z) * inv_z;
	  const Pos z_min_isec (ray.origin.x + ray.dir.x * z_min_scale,
				ray.origin.y + ray.dir.y * z_min_scale,
				z_min);
	  dist_t z_max_scale = (z_max - ray.origin.z) * inv_z;
	  const Pos z_max_isec (ray.origin.x + ray.dir.x * z_max_scale,
				ray.origin.y + ray.dir.y * z_max_scale,
				z_max);

	  // Get an IsecCache object.
	  //
	  Grab<IsecCache> isec_cache_grab (context.isec_cache_pool);

	  SearchState ss (*this, ray, callback, *isec_cache_grab);

	  ss.for_each_possible_intersector (0,
					    x_min_isec, x_max_isec,
					    y_min_isec, y_max_isec,
					    z_min_isec, z_max_isec);

	  ss.update_isec_stats (isec_stats);
	}
    }
}



// Ray intersection testing (Octree::SearchState::for_each_possible_intersector)

// Call our callback for each surface that intersects our ray in the
// octree underneath node NODE_INDEX.  The remaining parameters are
// pre-computed intersection points of the ray in the various planes
// bounding that node's volume.
//
// This method is critical for speed, and so we try to avoid doing any
// calculation at all.
//
void
Octree::SearchState::for_each_possible_intersector (
		       unsigned node_index,
		       const Pos &x_min_isec,
		       const Pos &x_max_isec,
		       const Pos &y_min_isec,
		       const Pos &y_max_isec,
		       const Pos &z_min_isec,
		       const Pos &z_max_isec)
{
  node_intersect_calls++;

  const Node &node = nodes[node_index];

  // The boundaries of our volume
  //
  const coord_t x_min = x_min_isec.x, x_max = x_max_isec.x;
  const coord_t y_min = y_min_isec.y, y_max = y_max_isec.y;
  const coord_t z_min = z_min_isec.z, z_max = z_max_isec.z;

  // Check to see if RAY intersects any of our faces.  Because we
  // already have the boundary-plane intersection points of RAY in the
  // ..._ISEC parameters, this requires only comparisons.  In the case
  // where RAY either starts or ends inside the volume, the
  // boundary-plane intersections are extensions of RAY, so we don't
  // need special cases for that occurance.
  //
  if (// RAY intersects x-min face
      //
      (x_min_isec.y >= y_min && x_min_isec.y <= y_max
       && x_min_isec.z >= z_min && x_min_isec.z <= z_max)
      //
      // RAY intersects x-max face
      //
      || (x_max_isec.y >= y_min && x_max_isec.y <= y_max
	  && x_max_isec.z >= z_min && x_max_isec.z <= z_max)
      //
      // RAY intersects y-min face
      //
      || (y_min_isec.x >= x_min && y_min_isec.x <= x_max
	  && y_min_isec.z >= z_min && y_min_isec.z <= z_max)
      //
      // RAY intersects y-max face
      //
      || (y_max_isec.x >= x_min && y_max_isec.x <= x_max
	  && y_max_isec.z >= z_min && y_max_isec.z <= z_max)
      //
      // RAY intersects z-min face
      //
      || (z_min_isec.x >= x_min && z_min_isec.x <= x_max
	  && z_min_isec.y >= y_min && z_min_isec.y <= y_max)
      //
      // RAY intersects z-max face
      //
      || (z_max_isec.x >= x_min && z_max_isec.x <= x_max
	  && z_max_isec.y >= y_min && z_max_isec.y <= y_max))
    {
      // RAY intersects some face, so it must intersect our volume

      // Invoke the callback on each of this node's surfaces
      //
      if (node.surface_ptrs_head_index)
	for (unsigned surf_ptr_index = node.surface_ptrs_head_index;
	     surface_ptrs[surf_ptr_index]; surf_ptr_index++)
	  {
	    const Surface *surf = surface_ptrs[surf_ptr_index];

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
	  // Calculate the mid-point intersections.  This the only real
	  // calculation we do in this method (hopefully dividing by two
	  // is efficient).
	  //
	  const Pos x_mid_isec = midpoint (x_min_isec, x_max_isec);
	  const Pos y_mid_isec = midpoint (y_min_isec, y_max_isec);
	  const Pos z_mid_isec = midpoint (z_min_isec, z_max_isec);
	  const coord_t x_mid = x_mid_isec.x;
	  const coord_t y_mid = y_mid_isec.y;
	  const coord_t z_mid = z_mid_isec.z;
	  const Pos &rbeg = ray.begin(), &rend = ray.end();

	  // Note that although RAY can actually change during the
	  // recursive calls below, it never will do so in a way that
	  // invalidates the factored-out bounds tests (it can get
	  // shorter, but never longer).

	  unsigned child_node_index; // used as a temporary below
	  if (rbeg.x <= x_mid || rend.x <= x_mid)
	    {
	      if (rbeg.y <= y_mid || rend.y <= y_mid)
		{
		  child_node_index
		    = node.child_node_indices[Node::X_LO|Node::Y_LO|Node::Z_LO];
		  if (child_node_index && (rbeg.z <= z_mid || rend.z <= z_mid))
		    for_each_possible_intersector (child_node_index,
						   x_min_isec, x_mid_isec,
						   y_min_isec, y_mid_isec,
						   z_min_isec, z_mid_isec);
		  if (unlikely (callback.stop)) return;

		  child_node_index
		    = node.child_node_indices[Node::X_LO|Node::Y_LO|Node::Z_HI];
		  if (child_node_index && (rbeg.z >= z_mid || rend.z >= z_mid))
		    for_each_possible_intersector (child_node_index,
						   x_min_isec, x_mid_isec,
						   y_min_isec, y_mid_isec,
						   z_mid_isec, z_max_isec);
		  if (unlikely (callback.stop)) return;
		}

	      if (rbeg.y >= y_mid || rend.y >= y_mid)
		{
		  child_node_index
		    = node.child_node_indices[Node::X_LO|Node::Y_HI|Node::Z_LO];
		  if (child_node_index && (rbeg.z <= z_mid || rend.z <= z_mid))
		    for_each_possible_intersector (child_node_index,
						   x_min_isec, x_mid_isec,
						   y_mid_isec, y_max_isec,
						   z_min_isec, z_mid_isec);
		  if (unlikely (callback.stop)) return;

		  child_node_index
		    = node.child_node_indices[Node::X_LO|Node::Y_HI|Node::Z_HI];
		  if (child_node_index && (rbeg.z >= z_mid || rend.z >= z_mid))
		    for_each_possible_intersector (child_node_index,
						   x_min_isec, x_mid_isec,
						   y_mid_isec, y_max_isec,
						   z_mid_isec, z_max_isec);
		  if (unlikely (callback.stop)) return;
		}
	    }

	  if (rbeg.x >= x_mid || rend.x >= x_mid)
	    {
	      if (rbeg.y <= y_mid || rend.y <= y_mid)
		{
		  child_node_index
		    = node.child_node_indices[Node::X_HI|Node::Y_LO|Node::Z_LO];
		  if (child_node_index && (rbeg.z <= z_mid || rend.z <= z_mid))
		    for_each_possible_intersector (child_node_index,
						   x_mid_isec, x_max_isec,
						   y_min_isec, y_mid_isec,
						   z_min_isec, z_mid_isec);
		  if (unlikely (callback.stop)) return;

		  child_node_index
		    = node.child_node_indices[Node::X_HI|Node::Y_LO|Node::Z_HI];
		  if (child_node_index && (rbeg.z >= z_mid || rend.z >= z_mid))
		    for_each_possible_intersector (child_node_index,
						   x_mid_isec, x_max_isec,
						   y_min_isec, y_mid_isec,
						   z_mid_isec, z_max_isec);
		  if (unlikely (callback.stop)) return;
		}

	      if (rbeg.y >= y_mid || rend.y >= y_mid)
		{
		  child_node_index
		    = node.child_node_indices[Node::X_HI|Node::Y_HI|Node::Z_LO];
		  if (child_node_index && (rbeg.z <= z_mid || rend.z <= z_mid))
		    for_each_possible_intersector (child_node_index,
						   x_mid_isec, x_max_isec,
						   y_mid_isec, y_max_isec,
						   z_min_isec, z_mid_isec);
		  if (unlikely (callback.stop)) return;

		  child_node_index
		    = node.child_node_indices[Node::X_HI|Node::Y_HI|Node::Z_HI];
		  if (child_node_index && (rbeg.z >= z_mid || rend.z >= z_mid))
		    for_each_possible_intersector (child_node_index,
						   x_mid_isec, x_max_isec,
						   y_mid_isec, y_max_isec,
						   z_mid_isec, z_max_isec);
		}
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
