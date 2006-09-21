// octree.cc -- Voxel tree datatype (hierarchically arranges 3D space)
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "octree.h"

using namespace Snogray;
using namespace std;


// Ray intersection testing

// Call CALLBACK for each surface in the voxel tree that _might_
// intersect RAY (any further intersection testing needs to be done
// directly on the resulting surfaces).
//
void
Octree::for_each_possible_intersector (const Ray &ray,
					IntersectCallback &callback)
  const
{
  if (root)
    {
      coord_t x_min = origin.x;
      coord_t x_max = origin.x + size;
      coord_t y_min = origin.y;
      coord_t y_max = origin.y + size;
      coord_t z_min = origin.z;
      coord_t z_max = origin.z + size;

      // First make sure RAY is conceivably within the top-most node
      const Pos &rbeg = ray.origin, &rend = ray.end();
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

	  root->for_each_possible_intersector (ray, callback,
					       x_min_isec, x_max_isec,
					       y_min_isec, y_max_isec,
					       z_min_isec, z_max_isec);
	}
    }
}

// Version of `for_each_possible_intersector' used for recursive
// voxel tree searching.  The additional parameters are pre-computed
// intersection points of the ray being intersected in the various
// planes bounding this node's volume (we don't actually need the
// ray itself).
//
// This method is critical for speed, and so we try to avoid doing any
// calculation at all.
//
void
Octree::Node::for_each_possible_intersector (const Ray &ray,
					      IntersectCallback &callback,
					      const Pos &x_min_isec,
					      const Pos &x_max_isec,
					      const Pos &y_min_isec,
					      const Pos &y_max_isec,
					      const Pos &z_min_isec,
					      const Pos &z_max_isec)
  const
{
  // The boundaries of our volume
  const coord_t x_min = x_min_isec.x, x_max = x_max_isec.x;
  const coord_t y_min = y_min_isec.y, y_max = y_max_isec.y;
  const coord_t z_min = z_min_isec.z, z_max = z_max_isec.z;

  if (callback.stats)
    callback.stats->node_intersect_calls++;

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

      // Invoke CALLBACK on each of this node's surfaces
      //
      for (list<Surface *>::const_iterator oi = surfaces.begin();
	   oi != surfaces.end(); oi++)
	{
	  callback (*oi);

	  if (callback.stop)
	    return;
	}

      // Recursively deal with any non-null sub-nodes
      //
      if (has_subnodes)
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
	  const Pos &rbeg = ray.origin, &rend = ray.end();

	  // Note that although RAY can actually change during the
	  // recursive calls below, it never will do so in a way that
	  // invalidates the factored-out bounds tests (it can get
	  // shorter, but never longer).

	  if (rbeg.x <= x_mid || rend.x <= x_mid)
	    {
	      if (rbeg.y <= y_mid || rend.y <= y_mid)
		{
		  if (x_lo_y_lo_z_lo && (rbeg.z <= z_mid || rend.z <= z_mid))
		    x_lo_y_lo_z_lo
		      ->for_each_possible_intersector (ray, callback,
						       x_min_isec, x_mid_isec,
						       y_min_isec, y_mid_isec,
						       z_min_isec, z_mid_isec);

		  if (x_lo_y_lo_z_hi && (rbeg.z >= z_mid || rend.z >= z_mid)
		      && !callback.stop)
		    x_lo_y_lo_z_hi
		      ->for_each_possible_intersector (ray, callback,
						       x_min_isec, x_mid_isec,
						       y_min_isec, y_mid_isec,
						       z_mid_isec, z_max_isec);
		}

	      if (rbeg.y >= y_mid || rend.y >= y_mid)
		{
		  if (x_lo_y_hi_z_lo && (rbeg.z <= z_mid || rend.z <= z_mid)
		      && !callback.stop)
		    x_lo_y_hi_z_lo
		      ->for_each_possible_intersector (ray, callback,
						       x_min_isec, x_mid_isec,
						       y_mid_isec, y_max_isec,
						       z_min_isec, z_mid_isec);

		  if (x_lo_y_hi_z_hi && (rbeg.z >= z_mid || rend.z >= z_mid)
		      && !callback.stop)
		    x_lo_y_hi_z_hi
		      ->for_each_possible_intersector (ray, callback,
						       x_min_isec, x_mid_isec,
						       y_mid_isec, y_max_isec,
						       z_mid_isec, z_max_isec);
		}
	    }

	  if (rbeg.x >= x_mid || rend.x >= x_mid)
	    {
	      if (rbeg.y <= y_mid || rend.y <= y_mid)
		{
		  if (x_hi_y_lo_z_lo && (rbeg.z <= z_mid || rend.z <= z_mid)
		      && !callback.stop)
		    x_hi_y_lo_z_lo
		      ->for_each_possible_intersector (ray, callback,
						       x_mid_isec, x_max_isec,
						       y_min_isec, y_mid_isec,
						       z_min_isec, z_mid_isec);

		  if (x_hi_y_lo_z_hi && (rbeg.z >= z_mid || rend.z >= z_mid)
		      && !callback.stop)
		    x_hi_y_lo_z_hi
		      ->for_each_possible_intersector (ray, callback,
						       x_mid_isec, x_max_isec,
						       y_min_isec, y_mid_isec,
						       z_mid_isec, z_max_isec);
		}

	      if (rbeg.y >= y_mid || rend.y >= y_mid)
		{
		  if (x_hi_y_hi_z_lo && (rbeg.z <= z_mid || rend.z <= z_mid)
		      && !callback.stop)
		    x_hi_y_hi_z_lo
		      ->for_each_possible_intersector (ray, callback,
						       x_mid_isec, x_max_isec,
						       y_mid_isec, y_max_isec,
						       z_min_isec, z_mid_isec);

		  if (x_hi_y_hi_z_hi && (rbeg.z >= z_mid || rend.z >= z_mid)
		      && !callback.stop)
		    x_hi_y_hi_z_hi
		      ->for_each_possible_intersector (ray, callback,
						       x_mid_isec, x_max_isec,
						       y_mid_isec, y_max_isec,
						       z_mid_isec, z_max_isec);
		}
	    }
	}
    }
}


// Octree construction

// Add SURFACE to the octree
//
void
Octree::add (Surface *surface, const BBox &surface_bbox)
{
  num_real_surfaces++;

  if (root)
    // We've already got some nodes.
    {
      // See if SURFACE fits...
      //
      if (origin.x <= surface_bbox.min.x
	  && origin.y <= surface_bbox.min.y
	  && origin.z <= surface_bbox.min.z
	  && (origin.x + size) >= surface_bbox.max.x
	  && (origin.y + size) >= surface_bbox.max.y
	  && (origin.z + size) >= surface_bbox.max.z)
	//
	// SURFACE fits within out root node, add it there, or in some sub-node
	//
	root->add (surface, surface_bbox, origin.x, origin.y, origin.z, size);
      else
	//
	// SURFACE doesn't fit within our root node, we have to make a new root
	//
	grow_to_include (surface, surface_bbox);
    }
  else
    // SURFACE will be the first node
    {
      root = new Node ();
      origin = surface_bbox.min;
      size = surface_bbox.max_size ();

#if 0
      cout << "made initial octree root at " << origin
	   << ", size = " << size << endl;
#endif

      // As we know that SURFACE will fit exactly in ROOT, we don't bother
      // calling ROOT's add method, we just add SURFACE directly to ROOT's
      // surface list.
      //
      root->surfaces.push_front (surface);
    }
}

// The current root of this octree is too small to encompass SURFACE;
// add surrounding levels of nodes until one can hold SURFACE, and make that
// the new root node.
//
void
Octree::grow_to_include (Surface *surface, const BBox &surface_bbox)
{
  // New root node
  //
  Node *new_root = new Node ();

  // Decide which directions to grow our volume
  //
  dist_t x_lo_grow = origin.x - surface_bbox.min.x;
  dist_t x_hi_grow = surface_bbox.max.x - (origin.x + size);
  dist_t y_lo_grow = origin.y - surface_bbox.min.y;
  dist_t y_hi_grow = surface_bbox.max.y - (origin.y + size);
  dist_t z_lo_grow = origin.z - surface_bbox.min.z;
  dist_t z_hi_grow = surface_bbox.max.z - (origin.z + size);

  // Install old root as appropriate sub-node of NEW_ROOT.
  //
  if (x_hi_grow > x_lo_grow)
    {					// grow in x-positive direction
      if (y_hi_grow > y_lo_grow)
	{				// grow in y-positive direction
	  if (z_hi_grow > z_lo_grow)	// grow in z-positive direction
	    new_root->x_lo_y_lo_z_lo = root;
	  else				// grow in z-negative direction
	    new_root->x_lo_y_lo_z_hi = root;
	}
      else
	{				// grow in y-negative direction
	  if (z_hi_grow > z_lo_grow)	// grow in z-positive direction
	    new_root->x_lo_y_hi_z_lo = root;
	  else				// grow in z-negative direction
	    new_root->x_lo_y_hi_z_hi = root;
	}
    }  
  else
    {					// grow in x-negative direction
      if (y_hi_grow > y_lo_grow)
	{				// grow in y-positive direction
	  if (z_hi_grow > z_lo_grow)	// grow in z-positive direction
	    new_root->x_hi_y_lo_z_lo = root;
	  else				// grow in z-negative direction
	    new_root->x_hi_y_lo_z_hi = root;
	}
      else
	{				// grow in y-negative direction
	  if (z_hi_grow > z_lo_grow)    // grow in z-positive direction
	    new_root->x_hi_y_hi_z_lo = root;
	  else				// grow in z-negative direction
	    new_root->x_hi_y_hi_z_hi = root;
	}
    }  

  // Adjust our position accordingly:  for each axis on which the old
  // root is installed in the "hi" slot, our old origin position now
  // becomes our new midpoint; for axes on which the old root is
  // installed in the "lo" slot, our origin remains the same.
  //
  if (x_hi_grow <= x_lo_grow)
    origin.x -= size;
  if (y_hi_grow <= y_lo_grow)
    origin.y -= size;
  if (z_hi_grow <= z_lo_grow)
    origin.z -= size;

  // Our size doubles with each new level.
  //
  size *= 2;

#if 0
  cout << "grew octree root to size: " << size << endl;
  cout << "   new origin is: " << origin << endl;
#endif

  new_root->has_subnodes = true;

  // Replace the old root!
  //
  root = new_root;

  // Now that we have a new root, try adding SURFACE again (if it still
  // doesn't fit, we'll be called again to add another level).
  //
  add (surface, surface_bbox);
}

// Add SURFACE, with bounding box SURFACE_BBOX, to this node or some subnode;
// SURFACE is assumed to fit.  X, Y, Z, and SIZE indicate the volume this
// node encompasses.
//
// This function is "eager": it splits empty nodes to find the smallest
// possible node for each new surface.  Not only does this simplify the
// algorithm, but it should also be more efficient for intersection
// testing -- testing whether a ray intersects a octree node for is a
// lot more efficient testing even simple surfaces, so the increased
// possibility of rejecting a ray without calling an surface's
// intersection routine is worth a fair number of levels of sparsely
// populated octree levels.
// 
void
Octree::Node::add (Surface *surface, const BBox &surface_bbox,
		    coord_t x, coord_t y, coord_t z, dist_t size)
{
  dist_t sub_size = size / 2;
  coord_t mid_x = x + sub_size, mid_y = y + sub_size, mid_z = z + sub_size;

  // See if SURFACE fits in some sub-node's volume, and if so, try to add
  // it there.

  // Start out assuming we'll add it at this level and set `add_here' to
  // false if we end up adding it to a subnode.
  //
  bool add_here = true;

  // If force_into_subnodes is true, we "force" an surface into multiple
  // subnodes even if it doesn't fit cleanly into any of them.  We do
  // this for oversized surfaces that straddle the volume midpoint,
  // taking a gamble that the risk of multiple calls to their
  // intersection method (because such forced surfaces will be present in
  // multiple subnodes) is outweighed by a much closer fit with the
  // descendent node they eventually end up in, allowing the octree to
  // reject more rays before reaching them.
  //
  bool force_into_subnodes = surface_bbox.avg_size() < size / 4;

  if (surface_bbox.max.x < mid_x
      || (surface_bbox.max.x == mid_x
	  && surface_bbox.min.x != surface_bbox.max.x)
      || (force_into_subnodes && surface_bbox.min.x < mid_x))
    {
      if (surface_bbox.max.y < mid_y
	  || (surface_bbox.max.y == mid_y
	      && surface_bbox.min.y != surface_bbox.max.y)
	  || (force_into_subnodes && surface_bbox.min.y < mid_y))
	{
	  if (surface_bbox.max.z < mid_z
	      || (surface_bbox.max.z == mid_z
		  && surface_bbox.min.z != surface_bbox.max.z)
	      || (force_into_subnodes && surface_bbox.min.z < mid_z))
	    {
	      add_or_create (x_lo_y_lo_z_lo, surface, surface_bbox,
			     x, y, z, sub_size);
	      add_here = false;
	    }
	  if (surface_bbox.min.z > mid_z
	      || (surface_bbox.min.z == mid_z
		  && surface_bbox.min.z != surface_bbox.max.z)
	      || (force_into_subnodes && surface_bbox.max.z > mid_z))
	    {
	      add_or_create (x_lo_y_lo_z_hi, surface, surface_bbox,
			     x, y, mid_z, sub_size);
	      add_here = false;
	    }
	}
      if (surface_bbox.min.y > mid_y
	  || (surface_bbox.min.y == mid_y
	      && surface_bbox.min.y != surface_bbox.max.y)
	  || (force_into_subnodes && surface_bbox.max.y > mid_y))
	{
	  if (surface_bbox.max.z < mid_z
	      || (surface_bbox.max.z == mid_z
		  && surface_bbox.min.z != surface_bbox.max.z)
	      || (force_into_subnodes && surface_bbox.min.z < mid_z))
	    {
	      add_or_create (x_lo_y_hi_z_lo, surface, surface_bbox,
			     x, mid_y, z, sub_size);
	      add_here = false;
	    }
	  if (surface_bbox.min.z > mid_z
	      || (surface_bbox.min.z == mid_z
		  && surface_bbox.min.z != surface_bbox.max.z)
	      || (force_into_subnodes && surface_bbox.max.z > mid_z))
	    {
	      add_or_create (x_lo_y_hi_z_hi, surface, surface_bbox,
			     x, mid_y, mid_z, sub_size);
	      add_here = false;
	    }
	}
    }
  if (surface_bbox.min.x > mid_x
      || (surface_bbox.min.x == mid_x
	  && surface_bbox.min.x != surface_bbox.max.x)
      || (force_into_subnodes && surface_bbox.max.x > mid_x))
    {
      if (surface_bbox.max.y < mid_y
	  || (surface_bbox.max.y == mid_y
	      && surface_bbox.min.y != surface_bbox.max.y)
	  || (force_into_subnodes && surface_bbox.min.y < mid_y))
	{
	  if (surface_bbox.max.z < mid_z
	      || (surface_bbox.max.z == mid_z
		  && surface_bbox.min.z != surface_bbox.max.z)
	      || (force_into_subnodes && surface_bbox.min.z < mid_z))
	    {
	      add_or_create (x_hi_y_lo_z_lo, surface, surface_bbox,
			     mid_x, y, z, sub_size);
	      add_here = false;
	    }
	  if (surface_bbox.min.z > mid_z
	      || (surface_bbox.min.z == mid_z
		  && surface_bbox.min.z != surface_bbox.max.z)
	      || (force_into_subnodes && surface_bbox.max.z > mid_z))
	    {
	      add_or_create (x_hi_y_lo_z_hi, surface, surface_bbox,
			     mid_x, y, mid_z, sub_size);
	      add_here = false;
	    }
	}
      if (surface_bbox.min.y > mid_y
	  || (surface_bbox.min.y == mid_y
	      && surface_bbox.min.y != surface_bbox.max.y)
	  || (force_into_subnodes && surface_bbox.max.y > mid_y))
	{
	  if (surface_bbox.max.z < mid_z
	      || (surface_bbox.max.z == mid_z
		  && surface_bbox.min.z != surface_bbox.max.z)
	      || (force_into_subnodes && surface_bbox.min.z < mid_z))
	    {
	      add_or_create (x_hi_y_hi_z_lo, surface, surface_bbox,
			     mid_x, mid_y, z, sub_size);
	      add_here = false;
	    }
	  if (surface_bbox.min.z > mid_z
	      || (surface_bbox.min.z == mid_z
		  && surface_bbox.min.z != surface_bbox.max.z)
	      || (force_into_subnodes && surface_bbox.max.z > mid_z))
	    {
	      add_or_create (x_hi_y_hi_z_hi, surface, surface_bbox,
			     mid_x, mid_y, mid_z, sub_size);
	      add_here = false;
	    }
	}
    }

  // If SURFACE didn't fit in any sub-node, add to this one
  //
  if (add_here)
    {
#if 0
      cout << "adding surface with bbox " << surface_bbox.min
	   << " - " << surface_bbox.max << endl
	   << "   to node @(" << x << ", " << y << ", " << z << ")" << endl
	   << "      size = " << size << endl
	   << "      prev num surfaces = " << surfaces.size() << endl;
#endif

      surfaces.push_back (surface);
    }
}



Octree::Node::~Node ()
{
  if (x_lo_y_lo_z_lo)
    delete x_lo_y_lo_z_lo;
  if (x_lo_y_lo_z_hi)
    delete x_lo_y_lo_z_hi;
  if (x_lo_y_hi_z_lo)
    delete x_lo_y_hi_z_lo;
  if (x_lo_y_hi_z_hi)
    delete x_lo_y_hi_z_hi;
  if (x_hi_y_lo_z_lo)
    delete x_hi_y_lo_z_lo;
  if (x_hi_y_lo_z_hi)
    delete x_hi_y_lo_z_hi;
  if (x_hi_y_hi_z_lo)
    delete x_hi_y_hi_z_lo;
  if (x_hi_y_hi_z_hi)
    delete x_hi_y_hi_z_hi;
}


// Statistics gathering
Space::Stats
Octree::stats () const
{
  Stats stats;
  if (root)
    root->upd_stats (stats);
  stats.num_dup_surfaces = stats.num_surfaces - num_real_surfaces;
  return stats;
}

// Update STATS to reflect this node.
//
void
Octree::Node::upd_stats (Stats &stats) const
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

  if (x_lo_y_lo_z_lo)
    num_subnodes++, x_lo_y_lo_z_lo->upd_stats (stats);
  if (x_lo_y_lo_z_hi)
    num_subnodes++, x_lo_y_lo_z_hi->upd_stats (stats);
  if (x_lo_y_hi_z_lo)
    num_subnodes++, x_lo_y_hi_z_lo->upd_stats (stats);
  if (x_lo_y_hi_z_hi)
    num_subnodes++, x_lo_y_hi_z_hi->upd_stats (stats);
  if (x_hi_y_lo_z_lo)
    num_subnodes++, x_hi_y_lo_z_lo->upd_stats (stats);
  if (x_hi_y_lo_z_hi)
    num_subnodes++, x_hi_y_lo_z_hi->upd_stats (stats);
  if (x_hi_y_hi_z_lo)
    num_subnodes++, x_hi_y_hi_z_lo->upd_stats (stats);
  if (x_hi_y_hi_z_hi)
    num_subnodes++, x_hi_y_hi_z_hi->upd_stats (stats);

  // Now update STATS

  // Num nodes
  //
  stats.num_nodes++;
  if (num_subnodes == 0)
    stats.num_leaf_nodes++;

  // Num surfaces
  //
  stats.num_surfaces += surfaces.size ();

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
