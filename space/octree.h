// octree.h -- Voxel tree datatype (hierarchically arranges 3D space)
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

#ifndef SNOGRAY_OCTREE_H
#define SNOGRAY_OCTREE_H

#include <list>

#include "geometry/pos.h"

#include "space.h"
#include "space-builder.h"


namespace snogray {


class Octree : public Space
{
public:

  // A class used for building a Space object.
  //
  class Builder;

  // Subclass of SpaceBuilderFactory for making octree builders.
  //
  class BuilderFactory;


  // Call CALLBACK for each surface in the voxel tree that _might_
  // intersect RAY (any further intersection testing needs to be done
  // directly on the resulting surfaces).  CONTEXT is used to access
  // various cache data structures.  ISEC_STATS will be updated.
  //
  virtual void for_each_possible_intersector (const Ray &ray,
					      IntersectCallback &callback,
					      RenderContext &context,
					      RenderStats::IsecStats &isec_stats)
    const;

  // Octree statistics.
  //
  struct Stats
  {
    Stats ()
      : num_nodes (0), num_leaf_nodes (0),
	num_surfaces (0), num_dup_surfaces (0),
	max_depth (0), avg_depth (0)
    { }

    unsigned long num_nodes;
    unsigned long num_leaf_nodes;
    unsigned long num_surfaces;
    unsigned long num_dup_surfaces;
    unsigned max_depth;
    float avg_depth;
  };

  // Return various statistics about this octree.
  //
  Stats stats () const;


private:

  // A octree node is one level of the tree, containing a cubic volume
  // (the size is not explicitly stored in the node).  It is divided
  // into 8 equally-sized sub-nodes by splitting the node equally along
  // each axis.
  //
  struct Node;

  // Class holding state during Octree searches.
  //
  struct SearchState;


  // Make a new, empty, octree with the given extent.  This should only
  // be invoked directly by Octree::Builder::make_space.
  //
  Octree (const Pos &_origin, dist_t _size);


  // Update STATS to reflect the node at index NODE_INDEX.
  //
  void upd_stats (const Node &node, Stats &stats) const;


  // Nodes in this octree.
  //
  std::vector<Node> nodes;

  // Pointers to surfaces referred to in this octree.
  // Surface-pointers occur in runs inside this vector with a NULL
  // pointer following the last entry in a list.
  //
  std::vector<const Surface *> surface_ptrs;

  // One corner of the octree.
  //
  Pos origin;

  // The size of the octree (in all dimensions).
  //
  dist_t size;

  // The number of "real" surfaces added to the octree.
  //
  unsigned long num_real_surfaces;
};



// Octree::BuilderFactory

// Subclass of SpaceBuilderFactory for making octree builders.
//
class Octree::BuilderFactory : public SpaceBuilderFactory
{
public:

  // Return a new SpaceBuilder object.
  //
  virtual SpaceBuilder *make_space_builder () const;
};


}

#endif // SNOGRAY_OCTREE_H


// arch-tag: 0b44a400-1a03-4967-ac84-a8984a4f2752
