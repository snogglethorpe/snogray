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
  // A callback for `for_each_possible_intersector'.  Users of
  // `for_each_possible_intersector' must subclass this, providing their
  // own operator() method, and adding any extra data fields they need.
  class FepiCallback
  {
  public:
    virtual bool operator() (const Obj *) = 0;
  };

  // Call CALLBACK for each object in the voxel tree that _might_
  // intersect RAY (any further intersection testing needs to be done
  // directly on the resulting objects).  If CALLBACK returns true, then
  // `for_each_possible_intersector' continues searching for possible
  // intersectors; otherwise, if CALLBACK returns false, then
  // `for_each_possible_intersector' immediately returns.
  bool for_each_possible_intersector (const Ray &ray, FepiCallback &callback)
    const;

  // One corner of the voxtree
  Pos origin;

  // The size of the voxtree (in all dimensions)
  Space::dist_t size;

private:  
  struct Node
  {
    // Version of `for_each_possible_intersector' used for recursive
    // voxel tree searching.  The additional parameters are pre-computed
    // intersection points of the ray being intersected in the various
    // planes bounding this node's volume (we don't actually need the
    // ray itself).
    bool for_each_possible_intersector (FepiCallback &callback,
					const Pos &x_min_isec,
					const Pos &x_max_isec,
					const Pos &y_min_isec,
					const Pos &y_max_isec,
					const Pos &z_min_isec,
					const Pos &z_max_isec)
      const;

    // Objects at this level of the tree.  All objects listed in a node
    // must fit entirely within it.  Any given object is only present in
    // a single node.
    std::list<const Obj *> objs;

    // The sub-nodes of this node; each sub-node is exactly half the
    // size of this node in all dimensions, so in total there are eight.
    Node *lower_left_front, *lower_right_front;
    Node *upper_left_front, *upper_right_front;
    Node *lower_left_back,  *lower_right_back;
    Node *upper_left_back,  *upper_right_back;

    // True if any of the above subnodes is non-null.
    bool has_subnodes;
  };

  // The root of the tree
  Node *root;
};

}

#endif /* __VOXTREE_H__ */

// arch-tag: 0b44a400-1a03-4967-ac84-a8984a4f2752
