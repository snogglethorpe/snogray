// voxtree.h -- Voxel tree datatype (hierarchically arranges 3D space)
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __VOXTREE_H__
#define __VOXTREE_H__

#include <list>

#include "pos.h"
#include "obj.h"

namespace Snogray {

class Voxtree
{
public:

  Voxtree () : root (0) { }

  // Add OBJ to the voxtree
  //
  void add (Obj *obj, const BBox &obj_bbox);
  void add (Obj *obj) { add (obj, obj->bbox ()); }

  struct Stats {
    Stats () : node_intersect_calls (0)
    { }
    unsigned long long node_intersect_calls;
  };

  // A callback for `for_each_possible_intersector'.  Users of
  // `for_each_possible_intersector' must subclass this, providing their
  // own operator() method, and adding any extra data fields they need.
  //
  struct IntersectCallback
  {
    IntersectCallback (Stats *_stats) : stop (false), stats (_stats) { }
    virtual ~IntersectCallback (); // stop gcc bitching

    virtual void operator() (Obj *) = 0;

    void stop_iteration () { stop = true; }

    // If set to true, return from iterator immediately
    bool stop;

    // This is used for stats gathering
    Stats *stats;
  };

  // Call CALLBACK for each object in the voxel tree that _might_
  // intersect RAY (any further intersection testing needs to be done
  // directly on the resulting objects).
  //
  void for_each_possible_intersector (const Ray &ray,
				      IntersectCallback &callback)
    const;

  unsigned num_nodes () const { return root ? root->num_nodes() : 0; }
  unsigned max_depth () const { return root ? root->max_depth() : 0; }

  // One corner of the voxtree
  Pos origin;

  // The size of the voxtree (in all dimensions)
  dist_t size;

private:  

  // The current root of this voxtree is too small to encompass OBJ;
  // add surrounding levels of nodes until one can hold OBJ, and make that
  // the new root node.
  //
  void grow_to_include (Obj *obj, const BBox &obj_bbox);

  // A voxtree node is one level of the tree, containing a cubic volume
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
					IntersectCallback &callback,
					const Pos &x_min_isec,
					const Pos &x_max_isec,
					const Pos &y_min_isec,
					const Pos &y_max_isec,
					const Pos &z_min_isec,
					const Pos &z_max_isec)
      const;

    // Add OBJ, with bounding box OBJ_BBOX, to this node or some subnode;
    // OBJ is assumed to fit.  X, Y, Z, and SIZE indicate the volume this
    // node encompasses.
    //
    void add (Obj *obj, const BBox &obj_bbox,
	      coord_t x, coord_t y, coord_t z, dist_t size);

    // A helper method that calls NODE's `add' method, after first
    // making sure that NODE exists (creating it if it does not).
    //
    void add_or_create (Node* &node, Obj *obj, const BBox &obj_bbox,
			       coord_t x, coord_t y, coord_t z, dist_t size)
    {
      if (! node)
	{
	  node = new Node ();
	  has_subnodes = true;
	}

      node->add (obj, obj_bbox, x, y, z, size);
    }
    
    unsigned num_nodes () const;
    unsigned max_depth (unsigned cur_sibling_max = 0) const;

    // Objects at this level of the tree.  All objects listed in a node
    // must fit entirely within it.  Any given object is only present in
    // a single node.
    //
    std::list<Obj *> objs;

    // The sub-nodes of this node; each sub-node is exactly half the
    // size of this node in all dimensions, so in total there are eight.
    //
    Node *x_lo_y_lo_z_lo, *x_lo_y_lo_z_hi, *x_lo_y_hi_z_lo, *x_lo_y_hi_z_hi;
    Node *x_hi_y_lo_z_lo, *x_hi_y_lo_z_hi, *x_hi_y_hi_z_lo, *x_hi_y_hi_z_hi;

    // True if any of the above subnodes is non-null.
    bool has_subnodes;
  };

  // The root of the tree
  Node *root;
};

}

#endif /* __VOXTREE_H__ */

// arch-tag: 0b44a400-1a03-4967-ac84-a8984a4f2752
