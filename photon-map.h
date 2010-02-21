// photon-map.h -- Data structure to hold photons in space
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __PHOTON_MAP_H__
#define __PHOTON_MAP_H__

#include <vector>

#include "snogmath.h"
#include "photon.h"


namespace snogray {


class BBox;


// A group of photons organized for fast spatial lookup.
//
class PhotonMap
{
public:

  // Set the photons in this PhotonMap to the photons in NEW_PHOTONS,
  // and build a kd-tree for them.  The contents of NEW_PHOTONS are
  // modified (but unreferenced afterwards, so may be discarded).
  //
  void set_photons (std::vector<Photon> &new_photons);

  // Search for the MAX_PHOTONS closest photons to POS.  Only photons
  // within a distance of MAX_DIST of POS are considered.
  //
  // Pointers to the photons found are inserted into the vector RESULTS,
  // in order of distance from POS.  RESULTS can never grow larger than
  // MAX_PHOTONS (but the photons in it will always be the closest
  // MAX_PHOTONS photons).
  //
  // If MAX_PHOTONS or more photons are found, returns the distance of
  // the farthest photon in RESULTS, _squared_, otherwise just returns
  // MAX_DIST*MAX_DIST.
  //
  dist_t find_photons (const Pos &pos, unsigned max_photons, dist_t max_dist,
		       std::vector<const Photon *> &results)
    const
  {
    dist_t max_dist_sq = max_dist * max_dist;
    if (! photons.empty ())
      find_photons (pos, 0, max_photons, max_dist_sq, results);
    // we could sort RESULTS here (cheaply if in heap form), but is it
    // worth it?
    return max_dist_sq;
  }

  // Return the number of photons in this map.
  //
  unsigned size () const { return photons.size (); }

  // Do a consistency check on the kd-tree data-structure.
  //
  void check_kd_tree ();

private:

  //
  // A kd-tree node describes a set of photons in a particular region of
  // space.  Each node splits one axis (x, y, or z) in space (the
  // "split-axis"), and has two sub-nodes, which hold only photons whose
  // position on that axis is less than (left) or greater-than to, the
  // "split-point" on the split-axis.
  //
  // Each kd-tree node is associated with a particular photon --
  // the median photon which determines the node's split-point.
  //
  // In this particular implementation, the "split-point" of a node is
  // the position on the node's split-axis of the median photon in that
  // node's sequence of photons, where the node's photons are sorted by
  // their position on the node's split-axis.
  //
  // As each node has an associated photon, and the only information
  // _not_ available in the photon is the split-axis of each node, we
  // just keep two vectors: an vector of photons, and a vector of
  // split-axes.  Both vectors are arranged as "left-balanced heaps":
  // the root node is at index 0, and for each node at index I, its
  // children are at indices 2*I+1 and 2*I+2.
  //

  // Copy photons from the source-range BEG to END, into the
  // PhotonMap::photons vector in kd-tree heap order, with the root at
  // index TARGET_INDEX (in PhotonMap::photons).  The ordering of
  // photons in the source range may be changed.
  //
  void make_kdtree (const std::vector<Photon>::iterator &beg,
		    const std::vector<Photon>::iterator &end,
		    unsigned target_index);

  // Search the kd-tree starting from the node at KD_TREE_NODE_INDEX,
  // for the MAX_PHOTONS closest photons to POS.  Only photons within a
  // distance of sqrt(MAX_DIST_SQ) of POS are considered.
  //
  // Pointers to the photons found are inserted into the vector RESULTS.
  // RESULTS can never grow larger than MAX_PHOTONS (but the photons in
  // it will always be the closest MAX_PHOTONS photons).
  //
  // The exact contents of RESULTS varies depending on its size:  If
  // RESULTS has fewer than MAX_PHOTONS elements, it will be an unsorted
  // ordinary vector, with new photons just added to the end; if it
  // contains MAX_PHOTONS elements, it will be a heap data structure
  // (see std::make_heap etc), and maintained in that form.
  //
  // MAX_DIST_SQ is an in/out parameter -- when RESULTS reaches its
  // maximum size (MAX_PHOTONS elements), then MAX_DIST_SQ will be
  // modified to be the most distance photon in RESULTS; this helps
  // prune the search by avoiding obviously too-distance parts of the
  // kd-tree.
  //
  void find_photons (const Pos &pos, unsigned kd_tree_node_index,
		     unsigned max_photons,
		     dist_t &max_dist_sq,
		     std::vector<const Photon *> &results)
    const;

  // Do a consistency check on the kd-tree data-structure.
  // Returns the number of nodes visited.
  //
  unsigned check_kd_tree (unsigned index, const BBox &bbox);

  // The actual photons.  There is one kd-tree node for each photon, and
  // each node's photon is the median split-point for that node.  The
  // photons are arranged as a left-balanced heap:  the root node is at
  // index 0, and for each node at index I, its children are at indices
  // 2*I+1 and 2*I+2.
  //
  std::vector<Photon> photons;

  // For each node in the kd-tree, the axis along which the node is
  // split (at the position of its median photon) to form child nodes.
  //
  std::vector<unsigned char> kd_tree_node_split_axes;
};


}

#endif // __PHOTON_MAP_H__
