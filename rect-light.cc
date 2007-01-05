// rect-light.cc -- Rectangular light
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "intersect.h"
#include "grid-iter.h"
#include "tripar-isec.h"

#include "rect-light.h"


using namespace snogray;


// Generate around NUM samples of this light and add them to SAMPLES.
// Return the actual number of samples (NUM is only a suggestion).
//
unsigned
RectLight::gen_samples (const Intersect &isec, unsigned num,
			IllumSampleVec &samples)
  const
{
  // First detect cases where the light isn't visible at all, by
  // examining the dot product of the surface normal with rays to the
  // four corners of the light.
  //
  if (isec.cos_n (pos - isec.pos) < 0
      && isec.cos_n (pos + side1 - isec.pos) < 0
      && isec.cos_n (pos + side2 - isec.pos) < 0
      && isec.cos_n (pos + side1 + side2 - isec.pos) < 0)
    return num;

  GridIter grid_iter (num);

  float u, v;
  while (grid_iter.next (u, v))
    {
      // Compute the position of the sample at U,V within the light.
      //
      const Pos s_end = pos + side1 * u + side2 * v;
      const Vec s_vec = s_end - isec.pos;

      if (isec.cos_n (s_vec) > 0)
	{
	  float dist = s_vec.length ();
	  float inv_dist = 1 / dist;
	  const Vec s_dir = s_vec * inv_dist;

	  // Area to solid-angle conversion, dw/dA
	  //   = cos (light_normal, -sample_dir) / distance^2
	  //
	  float dw_dA = fabs (dot (normal, s_dir)) * inv_dist * inv_dist;
	  float pdf = 1 / (area * dw_dA);

	  samples.push_back (IllumSample (s_dir, intensity, pdf, dist, this));
	}
    }

  return grid_iter.num_samples ();
}



// For every sample from BEG_SAMPLE to END_SAMPLE which intersects this
// light, and where light is closer than the sample's previously recorded
// light distance (or the previous distance is zero), overwrite the
// sample's light-related fields with information from this light.
//
void
RectLight::filter_samples (const Intersect &isec, 
			   const IllumSampleVec::iterator &beg_sample,
			   const IllumSampleVec::iterator &end_sample)
  const
{
  for (IllumSampleVec::iterator s = beg_sample; s != end_sample; s++)
    {
      double u, v;		// double to agree with
      dist_t dist
	= parallelogram_intersect (pos, side1, side2, isec.pos, s->dir, u, v);

      if (dist > 0 && (dist < s->dist || s->dist == 0))
	{
	  // Area to solid-angle conversion, dw/dA
	  //   = cos (light_normal, -sample_dir) / distance^2
	  //
	  float dw_dA = fabs (dot (normal, s->dir)) / (dist * dist);

	  // Pdf Is (1 / Area) * (Dw/Da)
	  //
	  s->light_pdf = 1 / (area * dw_dA);

	  s->val = intensity; //XXX * s->light_pdf;
	  s->dist = dist;
	  s->light = this;
	}
    }
}


// arch-tag: 60165b73-d34e-4f49-9a90-958daefdeb78
