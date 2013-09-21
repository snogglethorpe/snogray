// octree-node.h -- Node in an Octree
//
//  Copyright (C) 2005, 2007, 2009-2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_OCTREE_NODE_H
#define SNOGRAY_OCTREE_NODE_H

#include "octree.h"


namespace snogray {


// A octree node is one level of the tree, containing a cubic volume
// (the size is not explicitly stored in the node).  It is divided
// into 8 equally-sized sub-nodes by splitting the node equally along
// each axis.
//
struct Octree::Node
{
  // Constants for symbolic access to child node indices.  One each of
  // the X, Y, and Z constants may be or-ed together to form an index
  // in child_node_indices.
  //
  enum {
    X_LO = 0, X_HI = 4,
    Y_LO = 0, Y_HI = 2,
    Z_LO = 0, Z_HI = 1
  };

  Node ()
    : surface_ptrs_head_index (0)
  {
    for (unsigned i = 0; i < 8; i++)
      child_node_indices[i] = 0;
  }

  // Return true if this is a leaf node.  See the comment for the
  // child-index index fields below.
  //
  bool is_leaf_node () const
  {
    return child_node_indices[0] == 1 && child_node_indices[1] == 1;
  }

  // Mark this node as a leaf-node.  See the comment for the
  // child-index index fields below.
  //
  void mark_as_leaf_node ()
  {
    child_node_indices[0] = 1;
    child_node_indices[1] = 1;
  }

  // Indices of sub-nodes of this node in the Octree::nodes vector;
  // each sub-node is exactly half the size of this node in all
  // dimensions, so in total there are eight.
  //
  // A value of zero means "none" (the root node always has that
  // index).
  //
  // As a special case, if the first two entries have a value of 1, it
  // means "this is a leaf node."  [Note that such a state would
  // normally be invalid because a node cannot have the same child
  // twice.]  This special state only exists after an octree has
  // completely been setup.
  //
  unsigned child_node_indices[8];

  // Index of the first surface-pointer at this level of the tree in
  // the Octree::surface_ptrs vector; the list of pointers for a node
  // is terminated by a pointer with the low-bit set (but that last
  // pointer is still a valid pointer).  All surfaces listed in a node
  // must fit entirely within it.  Any given surface is only present
  // in a single node.
  //
  unsigned surface_ptrs_head_index;
};


}

#endif // SNOGRAY_OCTREE_NODE_H
