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

#include <algorithm>

#include "bbox.h"
#include "snogassert.h"

#include "photon-map.h"

using namespace snogray;




// Set the photons in this PhotonMap to the photons in NEW_PHOTONS, and
// build a kd-tree for them.  The contents of NEW_PHOTONS are modified
// (but unreferenced afterwards, so may be discarded).
//
void
PhotonMap::set_photons (std::vector<Photon> &new_photons)
{
  // Size the PHOTONS and KD_TREE_NODE_SPLIT_AXES vectors appropriately.
  //
  photons.resize (new_photons.size ());
  kd_tree_node_split_axes.resize (photons.size ());

  // Build the kdtree.
  //
  if (! photons.empty ())
    make_kdtree (new_photons.begin(), new_photons.end(), 0);
}


// left_balanced_left_child_nodes

// Return the number of nodes in the left child of a left-balanced
// tree with NUM total nodes.
//
// See J. A. Bærentzen, "On Left-balancing Binary Trees",
// Image Analysis & Computer Graphics, 2003-Aug
// http://www2.imm.dtu.dk/pubdb/p.php?2535
//
static unsigned
left_balanced_left_child_nodes (unsigned num)
{
  // Maximum number of nodes at the bottom level of the tree _if the
  // bottom level is completely filled_.
  //
  // A perfectly balanced binary tree will have 2^n - 1 nodes with
  // 2^(n - 1) leaf nodes in the bottom row, and 2^(n - 1) - 1 nodes
  // in the rest of the tree (the "non-bottom" nodes).
  //
  // For a tree with a number of nodes other than 2^n - 1, we can
  // still think of there being 2^(n - 1) - 1 non-bottom nodes and
  // fewer than 2^(n - 1) nodes in the bottom row.
  //
  unsigned max_bottom = 1;
  while (max_bottom * 2 <= num)
    max_bottom += max_bottom;

  // Number of nodes in all levels except the bottom.
  //
  unsigned non_bottom = max_bottom - 1;

  // Remainder of nodes, which must be stored in the bottom level.
  //
  unsigned bottom = num - non_bottom;

  // Maximum possible (leaf) nodes in the bottom row of the
  // left-child, if it were filled.
  //
  unsigned left_child_max_bottom = max_bottom / 2;

  // The number of non-bottom nodes in the left child, which is simply
  // the number of non-bottom nodes, minus 1 for this node, and then
  // divided by two (the right half will have the same number of
  // non-bottom nodes).
  //
  unsigned left_child_non_bottom = (non_bottom - 1) / 2;

  // The number of bottom nodes in the left child.  If the total
  // number of bottom nodes is less than half the maximum possible
  // bottom nodes, then all bottom nodes will be in the left child
  // (with the right-child having none); otherwise, the left child
  // will have all the bottom nodes it can hold, with the remainder
  // stored in the right-child.
  //
  unsigned left_child_bottom = min (bottom, left_child_max_bottom);

  // The total number of nodes in the left child is simply the sum of
  // the number of left-child non-bottom nodes and the number of
  // left-child bottom nodes.
  //
  return left_child_non_bottom + left_child_bottom;
}



// A comparison object for sorting vectors of photon along a given
// axis.
//
struct photon_axis_cmp
{
  photon_axis_cmp (unsigned _axis) : axis (_axis) { }

  bool operator() (const Photon &i, const Photon &j) const
  {
    return i.pos[axis] < j.pos[axis];
  }

  unsigned axis;
};

// Copy photons from the source-range BEG to END, into the
// PhotonMap::photons vector in kd-tree heap order, with the root at
// index TARGET_INDEX (in PhotonMap::photons).  The ordering of photons
// in the source range may be changed.
//
void
PhotonMap::make_kdtree (const std::vector<Photon>::iterator &beg,
			const std::vector<Photon>::iterator &end,
			unsigned target_index)
{
  // We always require at least a single photon range.
  //
  ASSERT (beg != end);

  // Make sure we're writing to a valid position in PhotonMap::photons.
  //
  ASSERT (target_index < photons.size());
    
  // The position of the median photon in our range.  This starts as
  // BEG to handle the leaf-node case; for other cases, the number of
  // children in the left-child will be added below.
  //
  std::vector<Photon>::iterator median = beg;

  // This will be the split-axis, the axis along which we split this
  // kd-tree node to form child nodes.
  //
  unsigned split_axis = 0;

  // If there's more than a single-photon in our range, find the best
  // axis to split along, re-arrange the photons in our range
  // accordingly, and make child kd-tree nodes.
  //
  if (beg + 1 != end)
    {
      // Find the bounding box of all the photons in our range.
      //
      // Note that we could avoid this calculation by passing the
      // bounding box as an argument during recursion, and shrinking it
      // to reflect splits, but re-calculating each time should yield
      // smaller bounding boxes, and shouldn't add significant run-time
      // -- it's O(beg-end), but so is our call to std::nth_element.
      //
      BBox bbox;
      for (std::vector<Photon>::iterator i = beg; i != end; ++i)
	bbox += i->pos;

      // Find the largest axis of the bounding-box, and make that our
      // split-axis.
      //
      dist_t max_bbox_dimen = 0;
      for (unsigned axis = 0; axis < 3; axis++)
	{
	  dist_t dimen = bbox.max[axis] - bbox.min[axis];
	  if (dimen > max_bbox_dimen)
	    {
	      max_bbox_dimen = dimen;
	      split_axis = axis;
	    }
	} 

      // Now add the number of photons in the the left-child to MEDIAN
      // (which started as BEG) get the true median position.
      //
      median += left_balanced_left_child_nodes (end - beg);

      // Now partition the photons in our range so the photon at
      // position MEDIAN is the the median photon in our range, on the
      // SPLIT_AXIS axis, and every photon from BEG to MEDIAN-1 has a
      // position less than the median photon (on the SPLIT_AXIS axis),
      // and every photon from MEDIAN+1 to END has a position greater
      // than the median photon.
      //
      std::nth_element (beg, median, end, photon_axis_cmp (split_axis));

      //
      // Now recursively call ourselves to arrange the photons in the
      // sub-sequences separated by MEDIAN.
      //

      // Left subtree:
      //
      if (median != beg)
	make_kdtree (beg, median, target_index * 2 + 1);

      // Right subtree:
      //
      if (median + 1 != end)
	make_kdtree (median + 1, end, target_index * 2 + 2);
    }
  
  // Copy the median photon to PhotonMap::photons[TARGET_INDEX], with
  // split-axis info added.
  //
  photons[target_index] = *median;
  kd_tree_node_split_axes[target_index] = split_axis;
}


// PhotonMap::find_photons

// A comparison object for sorting vectors of photon pointers
// according to their distance from a fixed point.
//
struct photon_ptr_dist_cmp
{
  photon_ptr_dist_cmp (const Pos &_pos) : pos (_pos) { }

  bool operator() (const Photon *i, const Photon *j) const
  {
    return
      (i->pos - pos).length_squared ()
      < (j->pos - pos).length_squared ();
  }

  const Pos &pos;
};

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
void
PhotonMap::find_photons (const Pos &pos, unsigned kd_tree_node_index,
			 unsigned max_photons,
			 dist_t &max_dist_sq,
			 std::vector<const Photon *> &results)
  const
{
  unsigned num_photons = photons.size ();

  const Photon &ph = photons[kd_tree_node_index];

  // First check child nodes in the kd-tree.
  //
  // The two child nodes have indices 2*i+1 and 2*i+2 (where i is this
  // node's index), so we can quickly check whether there are any
  // children just by see whether 2*i+2 lies within the allowable
  // indices.
  //
  if (kd_tree_node_index * 2 + 2 < num_photons)
    {
      unsigned split_axis = kd_tree_node_split_axes[kd_tree_node_index];

      // Position on the split-axis where this node splits it.
      //
      coord_t split_point = ph.pos[split_axis];

      // Distance along the split-axis between POS and SPLIT_POINT.
      //
      dist_t split_dist = pos[split_axis] - split_point;

      // kd-tree node indices of the first and second child nodes we'll search.
      //
      // We search the child which POS is within first, to allow better pruning.
      //
      unsigned first_child_index
	= kd_tree_node_index * 2 + ((split_dist < 0) ? 1 : 2);
      unsigned second_child_index
	= kd_tree_node_index * 2 + ((split_dist < 0) ? 2 : 1);

      // Search the first child.
      //
      find_photons (pos, first_child_index, max_photons, max_dist_sq,
		    results);

      // If POS is close enough to the split-point, search the second
      // child too.
      //
      if (split_dist * split_dist < max_dist_sq)
	find_photons (pos, second_child_index, max_photons, max_dist_sq,
		      results);
    }

  // Square of the distance between POS and PHOTON.
  //
  dist_t dist_sq = (pos - ph.pos).length_squared ();

  if (dist_sq < max_dist_sq)
    {
      photon_ptr_dist_cmp dist_cmp (pos);

      unsigned rsize = results.size();

      // If RESULTS is just 1 element short of being full, convert it
      // from an unsorted vector into a heap.
      //
      // otherwise, if RESULTS is full, first remove the farthest
      // photon from it (to be replaced by PHOTON).
      //
      if (rsize == max_photons - 1)
	{
	  std::make_heap (results.begin (), results.end (), dist_cmp);
	  rsize++;
	}
      else if (rsize == max_photons)
     	{
	  std::pop_heap (results.begin (), results.end (), dist_cmp);
	  results.pop_back ();
	}

      // Add a pointer to PHOTON to RESULTS.
      //
      results.push_back (&ph);

      // If RESULTS is full, maintain it in heap order.
      //
      if (rsize == max_photons)
	{
	  std::push_heap (results.begin (), results.end (), dist_cmp);

	  // Since we know we don't want anything more distance than
	  // what we've already found, update MAX_DIST_SQ (which an
	  // in/out parameter) to reflect the most distant photon in
	  // RESULTS.
	  //
	  max_dist_sq = (pos - results.front()->pos).length_squared ();
	}
    }
}


// PhotonMap::check_kd_tree

// Do a consistency check on the kd-tree data-structure.
//
void
PhotonMap::check_kd_tree ()
{
  BBox bbox;
  for (std::vector<Photon>::iterator i = photons.begin();
       i != photons.end(); ++i)
    bbox += i->pos;

  unsigned num = check_kd_tree (0, bbox);

  ASSERT (num == photons.size ());
}

// Do a consistency check on the kd-tree data-structure.
// All photons in this sub-tree must be within BBOX.
// Returns the number of nodes visited.
//
unsigned
PhotonMap::check_kd_tree (unsigned kd_tree_node_index, const BBox &bbox)
{
  if (kd_tree_node_index >= photons.size ())
    return 0;

  const Photon &ph = photons[kd_tree_node_index];

  unsigned split_axis = kd_tree_node_split_axes[kd_tree_node_index];
  ASSERT (split_axis < 3);	// unsigned, so always >= 0

  const Pos &pos = ph.pos;
  const Pos &min = bbox.min;
  const Pos &max = bbox.max;

  ASSERT (pos.x >= min.x && pos.y >= min.y && pos.z >= min.z);
  ASSERT (pos.x <= max.x && pos.y <= max.y && pos.z <= max.z);

  float split_point = pos[split_axis];

  BBox left_bbox = bbox;
  left_bbox.max[split_axis] = split_point;

  BBox right_bbox = bbox;
  right_bbox.min[split_axis] = split_point;

  unsigned lnum = check_kd_tree (kd_tree_node_index * 2 + 1, left_bbox);
  unsigned rnum = check_kd_tree (kd_tree_node_index * 2 + 2, right_bbox);

  return lnum + rnum + 1;
}

