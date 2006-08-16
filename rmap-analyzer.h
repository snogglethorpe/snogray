// rmap-analyzer.h -- Analyzer for dividing images into rectangular lights
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __RMAP_ANALYZER_H__
#define __RMAP_ANALYZER_H__

#include "radiance-map.h"
#include "image-sum.h"

#include "struct-light.h"


namespace Snogray {


// This is an abstract class for analyzing image-based light sources,
// for use with subclasses of StructLight.
//
class RmapAnalyzer : public StructLight::Analyzer
{
public:

  RmapAnalyzer (const RadianceMap &_rmap, float nominal_num_regions);

  // Return the radiance of the entire region (X, Y) - (X+W, Y+W)
  //
  Color radiance (float x, float y, float w, float h) const
  {
    return rmap_sum (unsigned (x), unsigned (y), unsigned (w), unsigned (h));
  }

  // Get the coordinates which bound the root.
  //
  virtual void get_root_bounds (float &x_min, float &y_min, float &w, float &h)
    const
  {
    x_min = 0;
    y_min = 0;
    w = width;
    h = height;
  }

  // Return true if the region (X, Y) - (X+W, Y+H) should be
  // split.  If true is returned, then the size and axis on which to
  // split are returned in SPLIT_POINT and SPLIT_DIM respectively.
  //
  virtual bool find_split_point (float x, float y, float w, float h,
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
    return radiance (x, y, w, h) / (w * h);
  }

  float width, height;

  const RadianceMap &rmap;

  const ImageSum rmap_sum;

  float nominal_region_area;

  float mean_intensity, inv_mean_intensity;
};


}

#endif /* __RMAP_ANALYZER_H__ */


// arch-tag: 6087c985-85ad-468c-8c70-74d528e3d4ba
