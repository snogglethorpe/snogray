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

// Call CALLBACK for each object in the voxel tree that _might_
// intersect RAY (any further intersection testing needs to be done
// directly on the resulting objects).  If CALLBACK returns true, then
// `for_each_possible_intersector' continues searching for possible
// intersectors; otherwise, if CALLBACK returns false, then
// `for_each_possible_intersector' immediately returns.
bool Voxtree::for_each_possible_intersector (const Ray &ray,
					     FepiCallback &callback)
  const
{
  if (! root)
    return true;

  // Compute the intersections of RAY with each of ROOT's bounding
  // planes.  Because ROOT's volume is aligned with the coordinate
  // axes, this is very simple, if a bit tedious.  Note that we
  // basically ignore the extent of RAY during these calculations,
  // and treat RAY as an infinite line.

  Space::coord_t x_min = origin.x;
  Space::dist_t x_min_scale = (x_min - ray.origin.x) / ray.dir.x;
  const Pos x_min_isec (x_min,
			ray.origin.y - ray.dir.y / x_min_scale,
			ray.origin.z - ray.dir.z / x_min_scale);
  Space::coord_t x_max = origin.x + size;
  Space::dist_t x_max_scale = (x_max - ray.origin.x) / ray.dir.x;
  const Pos x_max_isec (x_max,
			ray.origin.y - ray.dir.y / x_max_scale,
			ray.origin.z - ray.dir.z / x_max_scale);

  Space::coord_t y_min = origin.y;
  Space::dist_t y_min_scale = (y_min - ray.origin.y) / ray.dir.y;
  const Pos y_min_isec (ray.origin.x - ray.dir.x / y_min_scale,
			y_min,
			ray.origin.z - ray.dir.z / y_min_scale);
  Space::coord_t y_max = origin.y + size;
  Space::dist_t y_max_scale = (y_max - ray.origin.y) / ray.dir.y;
  const Pos y_max_isec (ray.origin.x - ray.dir.x / y_max_scale,
			y_max,
			ray.origin.z - ray.dir.z / y_max_scale);

  Space::coord_t z_min = origin.z;
  Space::dist_t z_min_scale = (z_min - ray.origin.z) / ray.dir.z;
  const Pos z_min_isec (ray.origin.x - ray.dir.x / z_min_scale,
			ray.origin.y - ray.dir.y / z_min_scale,
			z_min);
  Space::coord_t z_max = origin.z + size;
  Space::dist_t z_max_scale = (z_max - ray.origin.z) / ray.dir.z;
  const Pos z_max_isec (ray.origin.x - ray.dir.x / z_max_scale,
			ray.origin.y - ray.dir.y / z_max_scale,
			z_max);

  return root->for_each_possible_intersector (callback,
					      x_min_isec, x_max_isec,
					      y_min_isec, y_max_isec,
					      z_min_isec, z_max_isec);
}

// Version of `for_each_possible_intersector' used for recursive
// voxel tree searching.  The additional parameters are pre-computed
// intersection points of the ray being intersected in the various
// planes bounding this node's volume (we don't actually need the
// ray itself).
bool
Voxtree::Node::for_each_possible_intersector (Voxtree::FepiCallback &callback,
					      const Pos &x_min_isec,
					      const Pos &x_max_isec,
					      const Pos &y_min_isec,
					      const Pos &y_max_isec,
					      const Pos &z_min_isec,
					      const Pos &z_max_isec)
  const
{
  // The boundaries of our volume
  Space::coord_t x_min = x_min_isec.x, x_max = x_max_isec.x;
  Space::coord_t y_min = y_min_isec.y, y_max = y_max_isec.y;
  Space::coord_t z_min = z_min_isec.z, z_max = z_max_isec.z;

  // Check to see if RAY intersects any of our faces.  Because we
  // already have the boundary-plane intersection points of RAY in the
  // ..._ISEC parameters, this requires only comparisons.  In the case
  // where RAY either starts or ends inside the volume, the
  // boundary-plane intersections are extensions of RAY, so we don't
  // need special cases for that occurance.
  if (! (// left face
	 (x_min_isec.y >= y_min && x_min_isec.y <= y_max
	  && x_min_isec.z >= z_min && x_min_isec.z <= z_max)
	 // right face
	 || (x_max_isec.y >= y_max && x_max_isec.y <= y_max
	     && x_max_isec.z >= z_max && x_max_isec.z <= z_max)
	 // lower face
	 || (y_min_isec.x >= x_min && y_min_isec.x <= x_max
	     && y_min_isec.z >= z_min && y_min_isec.z <= z_max)
	 // upper face
	 || (y_max_isec.x >= x_max && y_max_isec.x <= x_max
	     && y_max_isec.z >= z_max && y_max_isec.z <= z_max)
	 // front face
	 || (y_min_isec.x >= x_min && y_min_isec.x <= x_max
	     && y_min_isec.z >= z_min && y_min_isec.z <= z_max)
	 // back face
	 || (y_max_isec.x >= x_max && y_max_isec.x <= x_max
	     && y_max_isec.z >= z_max && y_max_isec.z <= z_max)
	 ))
    // RAY intersects no face, so it must not intersect our volume;
    // give up (but continue searching in parent nodes).
    return true;

  // Invoke CALLBACK on each of this node's objects
  for (list<const Obj *>::const_iterator oi = objs.begin();
       oi != objs.end();
       oi++)
    if (! callback (*oi))
      // A false return from CALLBACK indicates that we should stop
      // (including in parent nodes).
      return false;

  // If there are no subnodes at all, give up early (but continue
  // searching in parent nodes).
  if (! has_subnodes)
    return true;

  // Recursively deal with any non-null sub-nodes

  // Calculate the mid-point intersections
  const Pos x_mid_isec = x_min_isec.midpoint (x_max_isec);
  const Pos y_mid_isec = y_min_isec.midpoint (y_max_isec);
  const Pos z_mid_isec = z_min_isec.midpoint (z_max_isec);

  // This gets set to false if the callback for any subnode returns false
  bool cont = true;

  if (lower_left_front && cont)
    cont
      = lower_left_front
          ->for_each_possible_intersector (callback,
					   x_min_isec, x_mid_isec,
					   y_min_isec, y_mid_isec,
					   z_min_isec, z_mid_isec);
  if (lower_right_front && cont)
    cont
      = lower_right_front
          ->for_each_possible_intersector (callback,
					   x_mid_isec, x_max_isec,
					   y_min_isec, y_mid_isec,
					   z_min_isec, z_mid_isec);
  if (upper_left_front && cont)
    cont
      = upper_left_front
          ->for_each_possible_intersector (callback,
					   x_min_isec, x_mid_isec,
					   y_mid_isec, y_max_isec,
					   z_min_isec, z_mid_isec);
  if (upper_right_front && cont)
    cont
      = upper_right_front
          ->for_each_possible_intersector (callback,
					   x_mid_isec, x_max_isec,
					   y_mid_isec, y_max_isec,
					   z_mid_isec, z_mid_isec);
  if (lower_left_back && cont)
    cont
      = lower_left_back
          ->for_each_possible_intersector (callback,
					   x_min_isec, x_mid_isec,
					   y_min_isec, y_mid_isec,
					   z_mid_isec, z_max_isec);
  if (lower_right_back && cont)
    cont
      = lower_right_back
          ->for_each_possible_intersector (callback,
					   x_mid_isec, x_max_isec,
					   y_min_isec, y_mid_isec,
					   z_mid_isec, z_max_isec);
  if (upper_left_back && cont)
    cont
      = upper_left_back
          ->for_each_possible_intersector (callback,
					   x_min_isec, x_mid_isec,
					   y_mid_isec, y_max_isec,
					   z_mid_isec, z_max_isec);
  if (upper_right_back && cont)
    cont
      = upper_right_back
          ->for_each_possible_intersector (callback,
					   x_mid_isec, x_max_isec,
					   y_mid_isec, y_max_isec,
					   z_mid_isec, z_max_isec);

  return cont;
}

// arch-tag: ec7b70cc-3cf6-40f3-9ec6-0ce71dbd20c5
