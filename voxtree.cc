// voxtree.cc -- Voxel tree datatype (hierarchically arranges 3D space)
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "voxtree.h"

using namespace Snogray;
using namespace std;


Voxtree::IntersectCallback::~IntersectCallback () { } // stop gcc bitching


// Ray intersection testing

// Call CALLBACK for each object in the voxel tree that _might_
// intersect RAY (any further intersection testing needs to be done
// directly on the resulting objects).
//
void
Voxtree::for_each_possible_intersector (const Ray &ray,
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

	  dist_t x_min_scale = (x_min - ray.origin.x) / ray.dir.x;
	  const Pos x_min_isec (x_min,
				ray.origin.y + ray.dir.y * x_min_scale,
				ray.origin.z + ray.dir.z * x_min_scale);
	  dist_t x_max_scale = (x_max - ray.origin.x) / ray.dir.x;
	  const Pos x_max_isec (x_max,
				ray.origin.y + ray.dir.y * x_max_scale,
				ray.origin.z + ray.dir.z * x_max_scale);

	  dist_t y_min_scale = (y_min - ray.origin.y) / ray.dir.y;
	  const Pos y_min_isec (ray.origin.x + ray.dir.x * y_min_scale,
				y_min,
				ray.origin.z + ray.dir.z * y_min_scale);
	  dist_t y_max_scale = (y_max - ray.origin.y) / ray.dir.y;
	  const Pos y_max_isec (ray.origin.x + ray.dir.x * y_max_scale,
				y_max,
				ray.origin.z + ray.dir.z * y_max_scale);

	  dist_t z_min_scale = (z_min - ray.origin.z) / ray.dir.z;
	  const Pos z_min_isec (ray.origin.x + ray.dir.x * z_min_scale,
				ray.origin.y + ray.dir.y * z_min_scale,
				z_min);
	  dist_t z_max_scale = (z_max - ray.origin.z) / ray.dir.z;
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
Voxtree::Node::for_each_possible_intersector (const Ray &ray,
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
      (x_min_isec.y >= y_min && x_min_isec.y <= y_max
       && x_min_isec.z >= z_min && x_min_isec.z <= z_max)
      // RAY intersects x-max face
      || (x_max_isec.y >= y_min && x_max_isec.y <= y_max
	  && x_max_isec.z >= z_min && x_max_isec.z <= z_max)
      // RAY intersects y-min face
      || (y_min_isec.x >= x_min && y_min_isec.x <= x_max
	  && y_min_isec.z >= z_min && y_min_isec.z <= z_max)
      // RAY intersects y-max face
      || (y_max_isec.x >= x_min && y_max_isec.x <= x_max
	  && y_max_isec.z >= z_min && y_max_isec.z <= z_max)
      // RAY intersects z-min face
      || (z_min_isec.x >= x_min && z_min_isec.x <= x_max
	  && z_min_isec.y >= y_min && z_min_isec.y <= y_max)
      // RAY intersects z-max face
      || (z_max_isec.x >= x_min && z_max_isec.x <= x_max
	  && z_max_isec.y >= y_min && z_max_isec.y <= y_max))
    {
      // RAY intersects some face, so it must intersect our volume

      // Invoke CALLBACK on each of this node's objects
      for (list<Obj *>::const_iterator oi = objs.begin();
	   oi != objs.end(); oi++)
	{
	  callback (*oi);

	  if (callback.stop)
	    return;
	}

      // Recursively deal with any non-null sub-nodes
      if (has_subnodes)
	{
	  // Calculate the mid-point intersections.  This the only real
	  // calculation we do in this method (hopefully dividing by two
	  // is efficient).
	  const Pos x_mid_isec = x_min_isec.midpoint (x_max_isec);
	  const Pos y_mid_isec = y_min_isec.midpoint (y_max_isec);
	  const Pos z_mid_isec = z_min_isec.midpoint (z_max_isec);
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


// Voxtree construction

// Add OBJ to the voxtree
//
void
Voxtree::add (Obj *obj, const BBox &obj_bbox)
{
  if (root)
    // We've already got some nodes.
    {
      // See if OBJ fits...
      if (origin.x <= obj_bbox.min.x
	  && origin.y <= obj_bbox.min.y
	  && origin.z <= obj_bbox.min.z
	  && (origin.x + size) >= obj_bbox.max.x
	  && (origin.y + size) >= obj_bbox.max.y
	  && (origin.z + size) >= obj_bbox.max.z)
	// OBJ fits within out root node, add it there, or in some sub-node
	root->add (obj, obj_bbox, origin.x, origin.y, origin.z, size);
      else
	// OBJ doesn't fit within our root node, we'll have to make a new root
	grow_to_include (obj, obj_bbox);
    }
  else
    // OBJ will be the first node
    {
      root = new Node ();
      origin = obj_bbox.min;
      size = obj_bbox.max_size ();

#if 0
      cout << "made initial voxtree root at " << origin
	   << ", size = " << size << endl;
#endif

      // As we know that OBJ will fit exactly in ROOT, we don't bother
      // calling ROOT's add method, we just add OBJ directly to ROOT's
      // object list.
      root->objs.push_front (obj);
    }
}

// The current root of this voxtree is too small to encompass OBJ;
// add surrounding levels of nodes until one can hold OBJ, and make that
// the new root node.
//
void
Voxtree::grow_to_include (Obj *obj, const BBox &obj_bbox)
{
  // New root node
  Node *new_root = new Node ();

  // Decide which directions to grow our volume
  dist_t x_lo_grow = origin.x - obj_bbox.min.x;
  dist_t x_hi_grow = obj_bbox.max.x - (origin.x + size);
  dist_t y_lo_grow = origin.y - obj_bbox.min.y;
  dist_t y_hi_grow = obj_bbox.max.y - (origin.y + size);
  dist_t z_lo_grow = origin.z - obj_bbox.min.z;
  dist_t z_hi_grow = obj_bbox.max.z - (origin.z + size);

  // Install old root as appropriate sub-node of NEW_ROOT.
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
  if (x_hi_grow <= x_lo_grow)
    origin.x -= size;
  if (y_hi_grow <= y_lo_grow)
    origin.y -= size;
  if (z_hi_grow <= z_lo_grow)
    origin.z -= size;

  // Our size doubles with each new level.
  size *= 2;

#if 0
  cout << "grew voxtree root to size: " << size << endl;
  cout << "   new origin is: " << origin << endl;
#endif

  new_root->has_subnodes = true;

  // Replace the old root!
  root = new_root;

  // Now that we have a new root, try adding OBJ again (if it still
  // doesn't fit, we'll be called again to add another level).
  add (obj, obj_bbox);
}

// Add OBJ, with bounding box OBJ_BBOX, to this node or some subnode;
// OBJ is assumed to fit.  X, Y, Z, and SIZE indicate the volume this
// node encompasses.
//
// This function is "eager": it splits empty nodes to find the smallest
// possible node for each new object.  Not only does this simplify the
// algorithm, but it should also be more efficient for intersection
// testing -- testing whether a ray intersects a voxtree node for is a
// lot more efficient testing even simple objects, so the increased
// possibility of rejecting a ray without calling an object's
// intersection routine is worth a fair number of levels of sparsely
// populated voxtree levels.
// 
void
Voxtree::Node::add (Obj *obj, const BBox &obj_bbox,
		    coord_t x, coord_t y, coord_t z, dist_t size)
{
  dist_t sub_size = size / 2;
  coord_t mid_x = x + sub_size, mid_y = y + sub_size, mid_z = z + sub_size;

  // See if OBJ fits in some sub-node's volume, and if so, try to add it there.

  // Start out assuming we'll add it at this level and set `add_here' to
  // false if we end up adding it to a subnode.
  bool add_here = true;

  // If force_into_subnodes is true, we "force" an object into multiple
  // subnodes even if it doesn't fit cleanly into any of them.  We do
  // this for oversized objects that straddle the volume midpoint,
  // taking a gamble that the risk of multiple calls to their
  // intersection method (because such forced objects will be present in
  // multiple subnodes) is outweighed by a much closer fit with the
  // descendent node they eventually end up in, allowing the voxtree to
  // reject more rays before reaching them.
  //
  bool force_into_subnodes = obj_bbox.avg_size() < size / 4;

  if (obj_bbox.max.x < mid_x
      || (obj_bbox.max.x == mid_x && obj_bbox.min.x != obj_bbox.max.x)
      || (force_into_subnodes && obj_bbox.min.x < mid_x))
    {
      if (obj_bbox.max.y < mid_y
	  || (obj_bbox.max.y == mid_y && obj_bbox.min.y != obj_bbox.max.y)
	  || (force_into_subnodes && obj_bbox.min.y < mid_y))
	{
	  if (obj_bbox.max.z < mid_z
	      || (obj_bbox.max.z == mid_z && obj_bbox.min.z != obj_bbox.max.z)
	      || (force_into_subnodes && obj_bbox.min.z < mid_z))
	    {
	      add_or_create (x_lo_y_lo_z_lo, obj, obj_bbox,
			     x, y, z, sub_size);
	      add_here = false;
	    }
	  if (obj_bbox.min.z >= mid_z
	      || (obj_bbox.min.z == mid_z && obj_bbox.min.z != obj_bbox.max.z)
	      || (force_into_subnodes && obj_bbox.max.z >= mid_z))
	    {
	      add_or_create (x_lo_y_lo_z_hi, obj, obj_bbox,
			     x, y, mid_z, sub_size);
	      add_here = false;
	    }
	}
      if (obj_bbox.min.y >= mid_y
	  || (obj_bbox.min.y == mid_y && obj_bbox.min.y != obj_bbox.max.y)
	  || (force_into_subnodes && obj_bbox.max.y >= mid_y))
	{
	  if (obj_bbox.max.z < mid_z
	      || (obj_bbox.max.z == mid_z && obj_bbox.min.z != obj_bbox.max.z)
	      || (force_into_subnodes && obj_bbox.min.z < mid_z))
	    {
	      add_or_create (x_lo_y_hi_z_lo, obj, obj_bbox,
			     x, mid_y, z, sub_size);
	      add_here = false;
	    }
	  if (obj_bbox.min.z >= mid_z
	      || (obj_bbox.min.z == mid_z && obj_bbox.min.z != obj_bbox.max.z)
	      || (force_into_subnodes && obj_bbox.max.z >= mid_z))
	    {
	      add_or_create (x_lo_y_hi_z_hi, obj, obj_bbox,
			     x, mid_y, mid_z, sub_size);
	      add_here = false;
	    }
	}
    }
  if (obj_bbox.min.x > mid_x
      || (obj_bbox.min.x == mid_x && obj_bbox.min.x != obj_bbox.max.x)
      || (force_into_subnodes && obj_bbox.max.x >= mid_x))
    {
      if (obj_bbox.max.y < mid_y
	  || (obj_bbox.max.y == mid_y && obj_bbox.min.y != obj_bbox.max.y)
	  || (force_into_subnodes && obj_bbox.min.y < mid_y))
	{
	  if (obj_bbox.max.z < mid_z
	      || (obj_bbox.max.z == mid_z && obj_bbox.min.z != obj_bbox.max.z)
	      || (force_into_subnodes && obj_bbox.min.z < mid_z))
	    {
	      add_or_create (x_hi_y_lo_z_lo, obj, obj_bbox,
			     mid_x, y, z, sub_size);
	      add_here = false;
	    }
	  if (obj_bbox.min.z >= mid_z
	      || (obj_bbox.min.z == mid_z && obj_bbox.min.z != obj_bbox.max.z)
	      || (force_into_subnodes && obj_bbox.max.z >= mid_z))
	    {
	      add_or_create (x_hi_y_lo_z_hi, obj, obj_bbox,
			     mid_x, y, mid_z, sub_size);
	      add_here = false;
	    }
	}
      if (obj_bbox.min.y >= mid_y
	  || (obj_bbox.min.y == mid_y && obj_bbox.min.y != obj_bbox.max.y)
	  || (force_into_subnodes && obj_bbox.max.y >= mid_y))
	{
	  if (obj_bbox.max.z < mid_z
	      || (obj_bbox.max.z == mid_z && obj_bbox.min.z != obj_bbox.max.z)
	      || (force_into_subnodes && obj_bbox.min.z < mid_z))
	    {
	      add_or_create (x_hi_y_hi_z_lo, obj, obj_bbox,
			     mid_x, mid_y, z, sub_size);
	      add_here = false;
	    }
	  if (obj_bbox.min.z >= mid_z
	      || (obj_bbox.min.z == mid_z && obj_bbox.min.z != obj_bbox.max.z)
	      || (force_into_subnodes && obj_bbox.max.z >= mid_z))
	    {
	      add_or_create (x_hi_y_hi_z_hi, obj, obj_bbox,
			     mid_x, mid_y, mid_z, sub_size);
	      add_here = false;
	    }
	}
    }

  // If OBJ didn't fit in any sub-node, add to this one
  if (add_here)
    {
#if 0
      cout << "adding object with bbox " << obj_bbox.min
	   << " - " << obj_bbox.max << endl
	   << "   to node @(" << x << ", " << y << ", " << z << ")" << endl
	   << "      size = " << size << endl
	   << "      prev num objs = " << objs.size() << endl;
#endif

      objs.push_back (obj);
    }
}



Voxtree::Node::~Node ()
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

unsigned
Voxtree::Node::num_nodes () const
{
  unsigned num = 1;
  if (x_lo_y_lo_z_lo)
    num += x_lo_y_lo_z_lo->num_nodes ();
  if (x_lo_y_lo_z_hi)
    num += x_lo_y_lo_z_hi->num_nodes ();
  if (x_lo_y_hi_z_lo)
    num += x_lo_y_hi_z_lo->num_nodes ();
  if (x_lo_y_hi_z_hi)
    num += x_lo_y_hi_z_hi->num_nodes ();
  if (x_hi_y_lo_z_lo)
    num += x_hi_y_lo_z_lo->num_nodes ();
  if (x_hi_y_lo_z_hi)
    num += x_hi_y_lo_z_hi->num_nodes ();
  if (x_hi_y_hi_z_lo)
    num += x_hi_y_hi_z_lo->num_nodes ();
  if (x_hi_y_hi_z_hi)
    num += x_hi_y_hi_z_hi->num_nodes ();

  return num;
}

unsigned
Voxtree::Node::max_depth (unsigned cur_sibling_max) const
{
  unsigned sub_max = 0;

  if (x_lo_y_lo_z_lo)
    sub_max = x_lo_y_lo_z_lo->max_depth (sub_max);
  if (x_lo_y_lo_z_hi)
    sub_max = x_lo_y_lo_z_hi->max_depth (sub_max);
  if (x_lo_y_hi_z_lo)
    sub_max = x_lo_y_hi_z_lo->max_depth (sub_max);
  if (x_lo_y_hi_z_hi)
    sub_max = x_lo_y_hi_z_hi->max_depth (sub_max);
  if (x_hi_y_lo_z_lo)
    sub_max = x_hi_y_lo_z_lo->max_depth (sub_max);
  if (x_hi_y_lo_z_hi)
    sub_max = x_hi_y_lo_z_hi->max_depth (sub_max);
  if (x_hi_y_hi_z_lo)
    sub_max = x_hi_y_hi_z_lo->max_depth (sub_max);
  if (x_hi_y_hi_z_hi)
    sub_max = x_hi_y_hi_z_hi->max_depth (sub_max);

  if (sub_max + 1> cur_sibling_max)
    return sub_max + 1;
  else
    return cur_sibling_max;
}

float
Voxtree::Node::avg_depth () const
{
  unsigned num_subnodes = 0;
  float subnode_sum = 0;

  if (x_lo_y_lo_z_lo)
    num_subnodes++, subnode_sum += x_lo_y_lo_z_lo->avg_depth ();
  if (x_lo_y_lo_z_hi)
    num_subnodes++, subnode_sum += x_lo_y_lo_z_hi->avg_depth ();
  if (x_lo_y_hi_z_lo)
    num_subnodes++, subnode_sum += x_lo_y_hi_z_lo->avg_depth ();
  if (x_lo_y_hi_z_hi)
    num_subnodes++, subnode_sum += x_lo_y_hi_z_hi->avg_depth ();
  if (x_hi_y_lo_z_lo)
    num_subnodes++, subnode_sum += x_hi_y_lo_z_lo->avg_depth ();
  if (x_hi_y_lo_z_hi)
    num_subnodes++, subnode_sum += x_hi_y_lo_z_hi->avg_depth ();
  if (x_hi_y_hi_z_lo)
    num_subnodes++, subnode_sum += x_hi_y_hi_z_lo->avg_depth ();
  if (x_hi_y_hi_z_hi)
    num_subnodes++, subnode_sum += x_hi_y_hi_z_hi->avg_depth ();

  if (num_subnodes == 0)
    return 1;
  else
    return (subnode_sum / num_subnodes) + 1;
}

unsigned
Voxtree::Node::num_objs () const
{
  unsigned num = objs.size ();

  if (x_lo_y_lo_z_lo)
    num += x_lo_y_lo_z_lo->num_objs ();
  if (x_lo_y_lo_z_hi)
    num += x_lo_y_lo_z_hi->num_objs ();
  if (x_lo_y_hi_z_lo)
    num += x_lo_y_hi_z_lo->num_objs ();
  if (x_lo_y_hi_z_hi)
    num += x_lo_y_hi_z_hi->num_objs ();
  if (x_hi_y_lo_z_lo)
    num += x_hi_y_lo_z_lo->num_objs ();
  if (x_hi_y_lo_z_hi)
    num += x_hi_y_lo_z_hi->num_objs ();
  if (x_hi_y_hi_z_lo)
    num += x_hi_y_hi_z_lo->num_objs ();
  if (x_hi_y_hi_z_hi)
    num += x_hi_y_hi_z_hi->num_objs ();
  
  return num;
}

// arch-tag: ec7b70cc-3cf6-40f3-9ec6-0ce71dbd20c5
