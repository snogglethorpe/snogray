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

#include "scene.h"

#include "struct-light.h"
#include "image.h"


using namespace Snogray;


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
	return slight.add_region (Region::U_SPLIT, split_point,
				  analyze (u, v, split_point, v_sz, slight),
				  analyze (u + split_point, v,
					   u_sz - split_point, v_sz,
					   slight));
      else
	return slight.add_region (Region::V_SPLIT, split_point,
				  analyze (u, v, u_sz, split_point, slight),
				  analyze (u, v + split_point,
					   u_sz, v_sz - split_point,
					   slight));
    }

  // We're not splitting this region, so add it as a leaf
  //
  return slight.add_region (radiance (u, v, u_sz, v_sz));
}



// Generate some samples from this region and its sub-regions, and
// add them to SAMPLES; N is the surface normal of the surface
// being illuminated.  The boundaries of the region are
// (U_MIN,V_MIN) - (U_MIN+U_SZ, V_MIN+V_SZ).
//
void
StructLight::Region::gen_samples (float u_min, float v_min,
				  float u_sz, float v_sz,
				  SampleRayVec &samples,
				  const StructLight *light)
  const
{
  switch (split_type)
    {
    case NO_SPLIT:
      if (radiance > Eps)
	{
	  float u = random (u_min, u_min + u_sz);
	  float v = random (v_min, v_min + v_sz);

	  u = (u - light->u_min) / light->u_sz;
	  v = (v - light->v_min) / light->v_sz;

	  // This is not really a valid sample:  we don't know the mapping
	  // from 2d uv space to 3d directions, so we simply encode the uv
	  // coordinates as the x and y components of the sample's
	  // direction vector.  The caller should complete the process by
	  // doing the conversion to a real direction.
	  //
	  samples.add_light (radiance, Vec (u, v, 0), 0, light);
	}
      break;

    case U_SPLIT:
      sub_region_0->gen_samples (u_min, v_min, split_point, v_sz,
				 samples, light);
      sub_region_1->gen_samples (u_min + split_point, v_min,
				 u_sz - split_point, v_sz, samples, light);
      break;

    case V_SPLIT:
      sub_region_0->gen_samples (u_min, v_min, u_sz, split_point,
				 samples, light);
      sub_region_1->gen_samples (u_min, v_min + split_point,
				 u_sz, v_sz - split_point, samples, light);
      break;
    }
}



void
StructLight::dump (const std::string &filename, const Image &orig_image) const
{
  Image image ((unsigned)u_sz, (unsigned)v_sz);

  float scale = (u_sz * v_sz) / (4 * M_PIf);

  if (root_region)
    root_region->dump (u_min, v_min, u_sz, v_sz, orig_image, scale, image);

  image.save (filename);
}

void
StructLight::Region::dump (float u_min, float v_min, float u_sz, float v_sz,
			   const Image &orig_image, float scale, Image &image)
  const
{
  switch (split_type)
    {
    case U_SPLIT:
      for (unsigned v = unsigned (v_min); v < unsigned (v_min + v_sz); v++)
	image.put (unsigned (u_min + split_point), image.height - v - 1,
		   Color (1, 0, 0));
      sub_region_0->dump (u_min, v_min, split_point, v_sz,
			  orig_image, scale, image);
      sub_region_1->dump (u_min + split_point, v_min, u_sz - split_point, v_sz,
			  orig_image, scale, image);
      break;

    case V_SPLIT:
      for (unsigned u = unsigned (u_min); u < unsigned (u_min + u_sz); u++)
	image.put (u, image.height - unsigned (v_min + split_point) - 1,
		   Color (1, 0, 0));
      sub_region_0->dump (u_min, v_min, u_sz, split_point,
			  orig_image, scale, image);
      sub_region_1->dump (u_min, v_min + split_point, u_sz, v_sz - split_point,
			  orig_image, scale, image);
      break;

    case NO_SPLIT:
      for (unsigned u = unsigned (u_min + 1); u < unsigned (u_min + u_sz); u++)
	for (unsigned v = unsigned (v_min + 1); v < unsigned(v_min + v_sz); v++)
	  if (u > unsigned (u_min + u_sz * 0.75)
	      && v > unsigned (v_min + v_sz * 0.75))
	    image.put (u, image.height - v - 1,
		       radiance * scale / (u_sz * v_sz));
	  else
	    image.put (u, image.height - v - 1, orig_image (u, v) * scale);
      break;
    }
}


// arch-tag: 602a40f7-344a-4a6a-85f8-b0165922179c
