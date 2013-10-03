// octree-builder.cc -- Octree construction
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

#include <deque>

#include "util/snogassert.h"

#include "octree.h"
#include "octree-node.h"


using namespace snogray;



// Octree::Builder

// A class used for building an Octree.
//
class Octree::Builder : public SpaceBuilder
{
public:

  Builder ()
    // surface_ptr_list_nodes is initialized with dummy entry
    : surface_ptr_list_nodes (1, SurfacePtrListNode (0,0)),
      num_real_surfaces (0)
  { }

  // Add SURFACE to the space being built.
  //
  virtual void add (const Surface *surface)
  {
    num_real_surfaces++;
    add (surface, surface->bbox ());
  }

  // Make the final space.  Note that this can only be done once.
  //
  virtual const Space *make_space ();

private:

  // An entry in a linked list of Surface pointers.  These are
  // referred to by integer indices (to make it possible to store them
  // in a growing vector).  Note that index 0 always means "end of
  // list."
  //
  // After octree building is complete, these linked lists are
  // unrolled into packed lists in the octree itself.
  //
  struct SurfacePtrListNode
  {
    SurfacePtrListNode (const Surface *_surface, unsigned _next_node_index)
      : surface (_surface), next_node_index (_next_node_index)
    { }
    const Surface *surface;
    unsigned next_node_index;
  };

  // Add SURFACE to the octree.  SURFACE_BBOX should be SURFACE's
  // bounding-box.
  //
  void add (const Surface *surface, const BBox &surface_bbox);

  // Add SURFACE, with bounding box SURFACE_BBOX, to the node at
  // NODE_INDEX or some subnode; SURFACE is assumed to fit.  X, Y, Z,
  // and SIZE indicate the volume this node encompasses.
  //
  void add (const Surface *surface, const BBox &surface_bbox,
	    unsigned node_index,
	    coord_t x, coord_t y, coord_t z, dist_t size);

  // Add SURFACE, with bounding box SURFACE_BBOX, to the child of the
  // node at NODE_INDEX selected by CHILD_NUM, or some subnode;
  // SURFACE is assumed to fit.  X, Y, Z, and SIZE indicate the volume
  // this node encompasses.
  //
  void add_to_child (const Surface *surface, const BBox &surface_bbox,
		     unsigned node_index, unsigned child_num,
		     coord_t x, coord_t y, coord_t z, dist_t size);

  // The current root of this octree is too small to encompass
  // SURFACE; add surrounding levels of nodes until one can hold
  // SURFACE, and make that the new root node.
  //
  void grow_to_include (const Surface *surface, const BBox &surface_bbox);

  // Push SURFACE onto the a list of surface-pointers whose head is
  // indicated by the index in HEAD_INDEX.  HEAD_INDEX is updated to
  // include the new pointer.
  //
  void push_surface_ptr (const Surface *surface, unsigned &head_index)
  {
    unsigned new_head = surface_ptr_list_nodes.size ();
    surface_ptr_list_nodes.push_back (SurfacePtrListNode (surface, head_index));
    head_index = new_head;
  }

  // Add the surface pointers in the linked-list whose head is at
  // HEAD_INDEX in Octree::Builder::surface_ptr_list_nodes, to the end
  // of SURFACE_PTRs, returning the index in SURFACE_PTRS of the first
  // entry (the the last entry will be at the end of SURFACE_PTRS).
  // An additional final zero entry is also added to terminate the
  // list.
  //
  unsigned unroll_surface_ptr_list (unsigned head_index,
				    std::vector<const Surface *> &surface_ptrs)
    const
  {
    unsigned rval = surface_ptrs.size ();
    for (unsigned index = head_index;
	 index; index = surface_ptr_list_nodes[index].next_node_index)
      surface_ptrs.push_back (surface_ptr_list_nodes[index].surface);
    surface_ptrs.push_back (0); // list terminator
    return rval;
  }

  // Copy all of our nodes into TO_NODES, and their associated surface
  // pointers into zero-terminated spans in TO_SURFACE_PTRS, using an
  // "optimized order", where nodes nearer the top of the node-tree
  // are closer to the front of TO_NODES (and the corresponding
  // surface lists are closer to beginning of TO_SURFACE_PTRS).
  //
  void copy_optimized_nodes (
	 std::vector<Node> &to_nodes,
	 std::vector<const Surface *> &to_surface_ptrs)
    const;

  // One corner of the octree.
  //
  Pos origin;

  // The size of the octree (in all dimensions).
  //
  dist_t size;

  // Nodes in the octree.
  //
  std::vector<Node> nodes;

  // Nodes in various linked-list of surface-pointers.  As index 0
  // always means "end of list," the first entry is a dummy value.
  //
  std::vector<SurfacePtrListNode> surface_ptr_list_nodes;

  // The number of "real" surfaces added to the octree.
  //
  unsigned long num_real_surfaces;
};



// Octree::Builder::add (from top-level)

// Add SURFACE to the octree.  SURFACE_BBOX should be SURFACE's
// bounding-box.
//
void
Octree::Builder::add (const Surface *surface, const BBox &surface_bbox)
{
  if (! nodes.empty ())
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
	add (surface, surface_bbox, 0,
	     origin.x, origin.y, origin.z, size);
      else
	//
	// SURFACE doesn't fit within our root node, we have to make a new root
	//
	grow_to_include (surface, surface_bbox);
    }
  else
    // SURFACE will be the first node
    {
      nodes.push_back (Node ());
      origin = surface_bbox.min;
      size = surface_bbox.max_size ();

#if 0
      cout << "made initial octree root at " << origin
	   << ", size = " << size << endl;
#endif

      // As we know that SURFACE will fit exactly in ROOT, we don't
      // bother calling ROOT's add method, we just add SURFACE
      // directly to ROOT's surface list.
      //
      push_surface_ptr (surface, nodes[0].surface_ptrs_head_index);
    }
}



// Octree::Builder::grow_to_include

// The current root of this octree is too small to encompass SURFACE;
// add surrounding levels of nodes until one can hold SURFACE, and
// make that the new root node.
//
void
Octree::Builder::grow_to_include (const Surface *surface,
				  const BBox &surface_bbox)
{
  // Make a new root node.  The root node must always be the first
  // entry in the Octree::nodes vector, but we must preserve the
  // position of other nodes so that their indices remain valid.  So
  // move the old root node to the end of Octree::nodes (there are no
  // references to it, so it's OK for its position to change).
  //
  unsigned old_root_index = nodes.size ();
  nodes.push_back (nodes[0]);	// move old root to end
  nodes[0] = Node ();		// initialize new root

  // Decide which directions to grow our volume
  //
  dist_t x_lo_grow = origin.x - surface_bbox.min.x;
  dist_t x_hi_grow = surface_bbox.max.x - (origin.x + size);
  dist_t y_lo_grow = origin.y - surface_bbox.min.y;
  dist_t y_hi_grow = surface_bbox.max.y - (origin.y + size);
  dist_t z_lo_grow = origin.z - surface_bbox.min.z;
  dist_t z_hi_grow = surface_bbox.max.z - (origin.z + size);

  // Install old root as appropriate sub-node of the new root.
  //
  if (x_hi_grow > x_lo_grow)
    {					// grow in x-positive direction
      if (y_hi_grow > y_lo_grow)
	{				// grow in y-positive direction
	  if (z_hi_grow > z_lo_grow)	// grow in z-positive direction
	    nodes[0].child_node_indices[Node::X_LO|Node::Y_LO|Node::Z_LO]
	      = old_root_index;
	  else				// grow in z-negative direction
	    nodes[0].child_node_indices[Node::X_LO|Node::Y_LO|Node::Z_HI]
	      = old_root_index;
	}
      else
	{				// grow in y-negative direction
	  if (z_hi_grow > z_lo_grow)	// grow in z-positive direction
	    nodes[0].child_node_indices[Node::X_LO|Node::Y_HI|Node::Z_LO] = old_root_index;
	  else				// grow in z-negative direction
	    nodes[0].child_node_indices[Node::X_LO|Node::Y_HI|Node::Z_HI] = old_root_index;
	}
    }
  else
    {					// grow in x-negative direction
      if (y_hi_grow > y_lo_grow)
	{				// grow in y-positive direction
	  if (z_hi_grow > z_lo_grow)	// grow in z-positive direction
	    nodes[0].child_node_indices[Node::X_HI|Node::Y_LO|Node::Z_LO]
	      = old_root_index;
	  else				// grow in z-negative direction
	    nodes[0].child_node_indices[Node::X_HI|Node::Y_LO|Node::Z_HI]
	      = old_root_index;
	}
      else
	{				// grow in y-negative direction
	  if (z_hi_grow > z_lo_grow)    // grow in z-positive direction
	    nodes[0].child_node_indices[Node::X_HI|Node::Y_HI|Node::Z_LO]
	      = old_root_index;
	  else				// grow in z-negative direction
	    nodes[0].child_node_indices[Node::X_HI|Node::Y_HI|Node::Z_HI]
	      = old_root_index;
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

  // Now that we have a new root, try adding SURFACE again (if it still
  // doesn't fit, we'll be called again to add another level).
  //
  add (surface, surface_bbox);
}



// Octree::Builder::add (general version)

// Add SURFACE, with bounding box SURFACE_BBOX, to the node at
// NODE_INDEX or some subnode; SURFACE is assumed to fit.  X, Y, Z,
// and SIZE indicate the volume this node encompasses.
//
// This function is "eager": it splits empty nodes to find the
// smallest possible node for each new surface.  Not only does this
// simplify the algorithm, but it should also be more efficient for
// intersection testing -- testing whether a ray intersects a octree
// node for is a lot more efficient testing even simple surfaces, so
// the increased possibility of rejecting a ray without calling an
// surface's intersection routine is worth a fair number of levels of
// sparsely populated octree levels.
//
void
Octree::Builder::add (const Surface *surface, const BBox &surface_bbox,
		      unsigned node_index,
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
	      add_to_child (surface, surface_bbox,
			    node_index, Node::X_LO|Node::Y_LO|Node::Z_LO,
			    x, y, z, sub_size);
	      add_here = false;
	    }
	  if (surface_bbox.min.z > mid_z
	      || (surface_bbox.min.z == mid_z
		  && surface_bbox.min.z != surface_bbox.max.z)
	      || (force_into_subnodes && surface_bbox.max.z > mid_z))
	    {
	      add_to_child (surface, surface_bbox,
			    node_index, Node::X_LO|Node::Y_LO|Node::Z_HI,
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
	      add_to_child (surface, surface_bbox,
			    node_index, Node::X_LO|Node::Y_HI|Node::Z_LO,
			    x, mid_y, z, sub_size);
	      add_here = false;
	    }
	  if (surface_bbox.min.z > mid_z
	      || (surface_bbox.min.z == mid_z
		  && surface_bbox.min.z != surface_bbox.max.z)
	      || (force_into_subnodes && surface_bbox.max.z > mid_z))
	    {
	      add_to_child (surface, surface_bbox,
			    node_index, Node::X_LO|Node::Y_HI|Node::Z_HI,
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
	      add_to_child (surface, surface_bbox,
			    node_index, Node::X_HI|Node::Y_LO|Node::Z_LO,
			    mid_x, y, z, sub_size);
	      add_here = false;
	    }
	  if (surface_bbox.min.z > mid_z
	      || (surface_bbox.min.z == mid_z
		  && surface_bbox.min.z != surface_bbox.max.z)
	      || (force_into_subnodes && surface_bbox.max.z > mid_z))
	    {
	      add_to_child (surface, surface_bbox,
			    node_index, Node::X_HI|Node::Y_LO|Node::Z_HI,
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
	      add_to_child (surface, surface_bbox,
			    node_index, Node::X_HI|Node::Y_HI|Node::Z_LO,
			    mid_x, mid_y, z, sub_size);
	      add_here = false;
	    }
	  if (surface_bbox.min.z > mid_z
	      || (surface_bbox.min.z == mid_z
		  && surface_bbox.min.z != surface_bbox.max.z)
	      || (force_into_subnodes && surface_bbox.max.z > mid_z))
	    {
	      add_to_child (surface, surface_bbox,
			    node_index, Node::X_HI|Node::Y_HI|Node::Z_HI,
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

      push_surface_ptr (surface, nodes[node_index].surface_ptrs_head_index);
    }
}



// Octree::Builder::add_to_child

// Add SURFACE, with bounding box SURFACE_BBOX, to the child of the
// node at NODE_INDEX selected by CHILD_NUM, or some subnode;
// SURFACE is assumed to fit.  X, Y, Z, and SIZE indicate the volume
// this node encompasses.
//
void
Octree::Builder::add_to_child (const Surface *surface, const BBox &surface_bbox,
			       unsigned node_index, unsigned child_num,
			       coord_t x, coord_t y, coord_t z, dist_t size)
{
  unsigned child_node_index = nodes[node_index].child_node_indices[child_num];

  // If this child hasn't been created yet, do so now.
  //
  if (! child_node_index)
    {
      // create the child
      child_node_index = nodes.size ();
      // make an empty node
      nodes.push_back (Node ());
      // record it in the parent
      nodes[node_index].child_node_indices[child_num] = child_node_index;
    }

  add (surface, surface_bbox, child_node_index, x, y, z, size);
}



// Octree::Builder::copy_optimized_nodes

// Copy all of our nodes into TO_NODES, and their associated surface
// pointers into zero-terminated spans in TO_SURFACE_PTRS, using an
// "optimized order", where nodes nearer the top of the node-tree are
// closer to the front of TO_NODES (and the corresponding surface
// lists are closer to beginning of TO_SURFACE_PTRS).
//
void
Octree::Builder::copy_optimized_nodes (
		   std::vector<Node> &to_nodes,
		   std::vector<const Surface *> &to_surface_ptrs)
  const
{
  //
  // Do initial setup of TO_NODES and TO_SURFACE_PTRS.
  //

  // Count the number of nodes that have surfaces, to use below in the
  // calculation of NUM_SURFACE_PTR_ENTRIES.
  //
  unsigned num_nodes_with_surfaces = 0;
  for (std::vector<Node>::const_iterator node = nodes.begin();
       node != nodes.end(); ++node)
    if (node->surface_ptrs_head_index)
      num_nodes_with_surfaces++;

  // The total number of surface pointers entries that we will use.
  //
  // Besides the number of actual surface pointers (which is the same
  // as the size of Octree::Builder::surface_ptr_list_nodes), we add
  // an extra zero-entry for each node with surfaces, to mark the end
  // of each node's surface-pointer list; this extra entry is not
  // explicitly stored in our linked-list form, but will be used in
  // the final packed form, so we need to account for it.
  //
  // [Both forms have a reserved zero entry, so we don't need to add that.]
  //
  unsigned num_surface_ptr_entries 
    = surface_ptr_list_nodes.size () + num_nodes_with_surfaces;

  // Pre-size OCTREE's vectors, for efficiency, and to avoid
  // over-allocation.
  //
  to_nodes.reserve (nodes.size ());
  to_surface_ptrs.reserve (num_surface_ptr_entries);

  // Add the reserved zero-entry to octree.surface_ptrs
  //
  to_surface_ptrs.push_back (0);


  //
  // Now copy nodes in FIFO order.
  //

  // FIFO queue of nodes to copy.
  //
  std::deque<unsigned> node_index_queue;

  // Index of the next free slot in the eventual nodes vector.
  //
  unsigned next_free_node_index = 1;

  // Prime queue with root node.
  //
  if (! nodes.empty ())
    node_index_queue.push_back (0);

  // Now copy nodes, continually getting the next node to copy from
  // the front of NODE_INDEX_QUEUE, and pushing its non-zero
  // child-node indices onto the back.  The result will be that all
  // nodes are copied in a breadth-first order.
  //
  while (! node_index_queue.empty ())
    {
      // Pop the next node to copy from the front of the queue.
      //
      unsigned from_index = node_index_queue.front ();
      node_index_queue.pop_front ();
      const Node &from_node = nodes[from_index];

      // The new node in TO_NODES we're copying to, which starts out empty.
      //
      unsigned to_index = to_nodes.size ();
      to_nodes.push_back (Node ());
      Node &to_node = to_nodes[to_index];

      // Copy FROM_NODE's surface pointers into TO_NODE, using contiguous
      // entries in TO_SURFACE_PTRS to hold them (rather than the linked
      // list used by FROM_NODE's surface-pointers).
      //
      if (from_node.surface_ptrs_head_index)
	to_node.surface_ptrs_head_index
	  = unroll_surface_ptr_list (from_node.surface_ptrs_head_index,
				     to_surface_ptrs);

      // Push the indices of sub-nodes of FROM_NODE onto the end of
      // NODE_INDEX_QUEUE, and mark TO_NODE as a leaf if there are no
      // sub-nodes
      //
      unsigned num_child_nodes = 0;
      for (unsigned i = 0; i < 8; i++)
	if (from_node.child_node_indices[i])
	  {
	    num_child_nodes++;

	    node_index_queue.push_back (from_node.child_node_indices[i]);

	    // Because we're copying in FIFO order, we know that each
	    // child node will be stored following nodes in
	    // NODE_INDEX_QUEUE.
	    //
	    to_node.child_node_indices[i] = next_free_node_index++;
	  }
      if (num_child_nodes == 0)
	to_node.mark_as_leaf_node ();
    }

  ASSERT (to_nodes.size () == nodes.size ());
  ASSERT (to_surface_ptrs.size () == num_surface_ptr_entries);
}



// Octree::Builder::make_space

// Make the final space.  Note that this can only be done once.
//
const Space *
Octree::Builder::make_space ()
{
  // Make a new octree, initially empty.
  //
  // Note that we don't invalidate our state, as it's actually still
  // valid (and could theoretically be used to make more octrees).  We
  // should be deleted soon anyway.
  //
  Octree *octree = new Octree (origin, size);

  // Copy the actual tree contents, in optimized order.
  //
  copy_optimized_nodes (octree->nodes, octree->surface_ptrs);

  // Now just fill in the remaining fields.
  //
  octree->num_real_surfaces = num_real_surfaces;

  return octree;
}



// Octree::BuilderFactory

// Return a new SpaceBuilder object.
//
SpaceBuilder *
Octree::BuilderFactory::make_space_builder () const
{
  return new Octree::Builder ();
}
