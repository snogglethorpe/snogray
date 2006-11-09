// lmap-analyzer.h -- Analyzer for dividing images into rectangular lights
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __LMAP_ANALYZER_H__
#define __LMAP_ANALYZER_H__

#include "light-map.h"
#include "image-sum.h"

#include "struct-light.h"


namespace Snogray {


// This is an abstract class for analyzing image-based light sources,
// for use with subclasses of StructLight.
//
class LmapAnalyzer : public StructLight::Analyzer
{
public:

  LmapAnalyzer (const LightMap &_lmap, float nominal_num_regions);

  // Return the light of the entire region (U, V) - (U+U_SZ, V+V_SZ)
  //
  Color intensity (float u, float v, float u_sz, float v_sz) const
  {
    return mean (u*width, v*height, u_sz*width, v_sz*height);
  }

  // Return true if the region (U, V) - (U+U_SZ, V+V_SZ) should be
  // split.  If true is returned, then the size and axis on which to
  // split are returned in SPLIT_POINT and SPLIT_DIM respectively.
  //
  virtual bool find_split_point (float u, float v, float u_sz, float v_sz,
				 float &split_point, SplitDim &split_dim)
    const;

private:

  // Return true if the region (X, Y) - (X+W, Y+W) should be split.
  //
  bool should_split (float x, float y, float w, float h) const;

  float find_dim_split_point (SplitDim split_dim,
			      float x, float y, float w, float h,
			      float &goodness)
    const;

  float judge_split (float split, SplitDim split_dim,
		     float x, float y, float w, float h)
    const;

  // Return the mean color of the entire region (X, Y) - (X+W, Y+H)
  //
  Color mean (float x, float y, float w, float h) const
  {
    return lmap_sum.average (unsigned (x), unsigned (y),
			     unsigned (w), unsigned (h));
  }

  float width, height;

  const LightMap &lmap;

  const ImageSum lmap_sum;

  float nominal_region_area;

  float mean_intensity, inv_mean_intensity;
};


}

#endif /* __LMAP_ANALYZER_H__ */


// arch-tag: 6087c985-85ad-468c-8c70-74d528e3d4ba
