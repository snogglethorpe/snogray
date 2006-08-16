// rmap-analyzer.cc -- Analyzer for dividing images into rectangular lights
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "snogmath.h"

#include "rmap-analyzer.h"


using namespace Snogray;


// If X is > 1, return 1 / X, else return X.
//
static float maybe_inv (float x) { return x > 1 ? 1 / x : x; }


RmapAnalyzer::RmapAnalyzer (const RadianceMap &_rmap, float nominal_num_regions)
  : width (_rmap.width), height (_rmap.height),
    rmap (_rmap), rmap_sum  (*_rmap.map),
    nominal_region_area (width * height / nominal_num_regions),
    mean_intensity (mean (0, 0, width, height).intensity ()),
    inv_mean_intensity (1 / mean_intensity)
{ }



// Return true if the region (X, Y) - (X+W, Y+W) should be split.
//
bool
RmapAnalyzer::should_split (float x, float y, float w, float h)
  const
{
  if (rmap.too_small (x, y, w, h))
    return false;
  else if (rmap.too_big (x, y, w, h))
    return true;
  else
    {
      float area = w * h;

      float intens_dev = mean (x, y, w, h).intensity() * inv_mean_intensity;
      float ar = maybe_inv (rmap.aspect_ratio (x, y, w, h));

      if (intens_dev == 0)
	return false;

      float desired_area = nominal_region_area;
      desired_area *= ar;
      desired_area /= intens_dev;

      return area > desired_area;
    }
}



float
RmapAnalyzer::judge_split (float split, SplitDim split_dim,
			   float x, float y, float w, float h)
  const
{
  float x1 = x;
  float x2 = (split_dim == U_DIM) ? x + split : x;
  float y1 = y;
  float y2 = (split_dim == V_DIM) ? y + split : y;
  float w1 = (split_dim == U_DIM) ? split : w;
  float w2 = (split_dim == U_DIM) ? w - split : w;
  float h1 = (split_dim == V_DIM) ? split : h;
  float h2 = (split_dim == V_DIM) ? h - split : h;

  // We mainly judge the split based on the average goodness of our
  // childrens' aspect ratios.  This may seem a bit simplistic, but it's
  // important to get even coverage in the case that the (rectangular)
  // radiance-map is mapped on to a sphere for environment-map lighting.

  float ar1 = maybe_inv (rmap.aspect_ratio (x1, y1, w1, h1));
  float ar2 = maybe_inv (rmap.aspect_ratio (x2, y2, w2, h2));

  float child_ar_av = (ar1 + ar2) * 0.5f;

#if 0

  // For brighter-than average areas, we also use a very simple sort of
  // edge-detection so that regions slightly better reflect the geometry of
  // the image.  [We don't do it in dim regions, as it tends to misbehave.]

  if (mean (x, y, w, h) > mean_intensity * 2)
    {
      float mean1 = mean (x1, y1, w1, h1).intensity ();
      float mean2 = mean (x2, y2, w2, h2).intensity ();
      float mean_diff
	= (mean1 + mean2) == 0 ? 0 : abs (mean1 - mean2) / (mean1 + mean2);

      return (child_ar_av * 0.95f) + (mean_diff * 0.05f);
    }
  else
    return child_ar_av;

#else

  return child_ar_av;

#endif
}



#define GOODNESS_EPS 	0.01

float
RmapAnalyzer::find_dim_split_point (SplitDim split_dim,
				     float x, float y, float w, float h,
				     float &goodness)
  const
{
  float sz = (split_dim == U_DIM) ? w : h;

  float center = floor (sz / 2);
  goodness = judge_split (center, split_dim, x, y, w, h);

  float split = center;

  for (float try_split = center + 1; try_split < sz * 0.75; try_split++)
    {
      float new_goodness = judge_split (try_split, split_dim, x, y, w, h);
      if (new_goodness > goodness + GOODNESS_EPS)
	{
	  split = try_split;
	  goodness = new_goodness;
	}
    }

  for (float try_split = center - 1; try_split > sz * 0.25; try_split--)
    {
      float new_goodness = judge_split (try_split, split_dim, x, y, w, h);
      if (new_goodness > goodness + GOODNESS_EPS)
	{
	  split = try_split;
	  goodness = new_goodness;
	}
    }

  return split;
}

// Return true if the region (X, Y) - (X+W, Y+H) should be
// split.  If true is returned, then the size and axis on which to
// split are returned in SPLIT_POINT and SPLIT_DIM respectively.
//
bool
RmapAnalyzer::find_split_point (float x, float y, float w, float h,
				 float &split_point, SplitDim &split_dim)
  const
{
  if (should_split (x, y, w, h))
    {
      float x_goodness, y_goodness;
      float x_split = find_dim_split_point (U_DIM, x, y, w, h, x_goodness);
      float y_split = find_dim_split_point (V_DIM, x, y, w, h, y_goodness);

      if (x_goodness > y_goodness)
	{
	  split_point = x_split;
	  split_dim = U_DIM;
	}
      else
	{
	  split_point = y_split;
	  split_dim = V_DIM;
	}

      return true;
    }
  else
    return false;
}


// arch-tag: e04ccc9f-9797-4573-8fae-46c2bce62e10
