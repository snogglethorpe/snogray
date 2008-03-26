// struct-light.cc -- A structured light source
//
//  Copyright (C) 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//


#include <algorithm>

#include "scene.h"

#include "struct-light.h"
#include "image.h"


using namespace snogray;



void
StructLight::analyze (const Analyzer &analyzer)
{
  // should free any non-null root_region  XXX

  root_region = analyzer.analyze (*this);

  num_sample_regions = float (sample_regions.size ());

  if (num_sample_regions != 0)
    inv_num_sample_regions = 1 / num_sample_regions;
  else
    inv_num_sample_regions = 0;
}



// Get some statistics about this light.
//
void
StructLight::get_stats (unsigned &num_sample_regions,
			unsigned &num_leaf_regions,
			Color &mean_intensity)
  const
{
  if (root_region)
    {
      num_sample_regions = sample_regions.size ();

      mean_intensity = 0;
      num_leaf_regions = 0;

      for (std::vector<Region *>::const_iterator r = sample_regions.begin ();
	   r != sample_regions.end (); ++r)
	{
	  mean_intensity += (*r)->intensity / (*r)->pdf_weight;

	  if (r == sample_regions.begin() || *r != *(r-1))
	    num_leaf_regions++;
	}
    }
  else
    {
      num_sample_regions = 0;
      num_leaf_regions = 0;
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
  float split_point, leaf_weight;
  SplitDim split_dim;

  if (find_split_point (u, v, u_sz, v_sz, split_dim, split_point, leaf_weight))
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
  return slight.add_region (intensity (u, v, u_sz, v_sz), leaf_weight,
			    u, v, u_sz, v_sz);
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
	    image (x, image.height - y - 1) = intensity;
	  else
	    image (x, image.height - y - 1) = orig_image (x, y);
    }
  else
    {
      sub_region_0->dump (orig_image, image);
      sub_region_1->dump (orig_image, image);

      Color div_color (1, 0, 0);

      // Draw divider lines (overwriting part of the image).
      //
      if (kind == U_SPLIT)
	{
	  unsigned x_split
	    = unsigned (float (image.width) * sub_region_1->u_min + 0.5f);
	  for (unsigned y = y_min; y < y_lim; y++)
	    image (x_split, image.height - y - 1) = div_color;
	}
      else
	{
	  unsigned y_split
	    = unsigned (float (image.height) * sub_region_1->v_min + 0.5f);
	  for (unsigned x = x_min; x < x_lim; x++)
	    image (x, image.height - y_split - 1) = div_color;
	}
    }
}


// arch-tag: 602a40f7-344a-4a6a-85f8-b0165922179c
