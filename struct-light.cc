// struct-light.cc -- A structured light source
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <algorithm>

#include "scene.h"

#include "struct-light.h"
#include "image.h"


using namespace Snogray;



void
StructLight::analyze (const Analyzer &analyzer)
{
  // should free any non-null root_region  XXX

  root_region = analyzer.analyze (*this);

  if (! leaf_regions.empty ())
    {
      num_leaf_regions = float (leaf_regions.size ());
      inv_num_leaf_regions = 1 / num_leaf_regions;
    }
}



// Get some statistics about this light.
//
void
StructLight::get_stats (unsigned &num_regions, Color &mean_intensity) const
{
  if (root_region)
    {
      num_regions = leaf_regions.size ();

      mean_intensity = 0;
      for (std::vector<Region *>::const_iterator r = leaf_regions.begin ();
	   r != leaf_regions.end (); ++r)
	mean_intensity += (*r)->intensity * (*r)->area;
    }
  else
    {
      num_regions = 0;
      mean_intensity = 0;
    }    
}



// Analyzes the structure and returns a region-tree covering it.
//
StructLight::Region *
StructLight::Analyzer::analyze (float u, float v, float u_sz, float v_sz,
				StructLight &slight)
  const
{
  float split_point;
  SplitDim split_dim;

  if (find_split_point (u, v, u_sz, v_sz, split_point, split_dim))
    //
    // Try splitting
    {
      if (split_dim == U_DIM)
	return slight.add_region (Region::U_SPLIT,
				  analyze (u, v, split_point, v_sz, slight),
				  analyze (u + split_point, v,
					   u_sz - split_point, v_sz,
					   slight));
      else
	return slight.add_region (Region::V_SPLIT,
				  analyze (u, v, u_sz, split_point, slight),
				  analyze (u, v + split_point,
					   u_sz, v_sz - split_point,
					   slight));
    }

  // We're not splitting this region, so add it as a leaf
  //
  return slight.add_region (intensity (u, v, u_sz, v_sz), u, v, u_sz, v_sz);
}


// Sample map dumping

// Dump a picture of the generated light regions to a file called
// FILENAME.  ORIG_IMAGE should be the original image from which this
// light was created.
//
void
StructLight::dump (const std::string &filename, const Image &orig_image) const
{
  Image image (orig_image.width, orig_image.height);

  if (root_region)
    root_region->dump (orig_image, image);

  image.save (filename);
}

void
StructLight::Region::dump (const Image &orig_image, Image &image)
  const
{
  unsigned x_min = unsigned (float (image.width)  * u_min + 0.5f);
  unsigned y_min = unsigned (float (image.height) * v_min + 0.5f);
  unsigned x_lim = unsigned (float (image.width)  * (u_min + u_sz) + 0.5f);
  unsigned y_lim = unsigned (float (image.height) * (v_min + v_sz) + 0.5f);

  if (kind == LEAF)
    {
      for (unsigned x = x_min; x < x_lim; x++)
	for (unsigned y = y_min; y < y_lim; y++)
	  if (x > (x_min + 3 * x_lim) / 4 && y > (y_min + 3 * y_lim) / 4)
	    image.put (x, image.height - y - 1, intensity);
	  else
	    image.put (x, image.height - y - 1, orig_image (x, y));
    }
  else
    {
      sub_region_0->dump (orig_image, image);
      sub_region_1->dump (orig_image, image);

      // Draw divider lines (overwriting part of the image).
      //
      if (kind == U_SPLIT)
	{
	  unsigned x_split
	    = unsigned (float (image.width) * sub_region_1->u_min + 0.5f);
	  for (unsigned y = y_min; y < y_lim; y++)
	    image.put (x_split, image.height - y - 1, Color (1, 0, 0));
	}
      else
	{
	  unsigned y_split
	    = unsigned (float (image.height) * sub_region_1->v_min + 0.5f);
	  for (unsigned x = x_min; x < x_lim; x++)
	    image.put (x, image.height - y_split - 1, Color (1, 0, 0));
	}
    }
}


// arch-tag: 602a40f7-344a-4a6a-85f8-b0165922179c
