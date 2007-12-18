// far-light.cc -- Light at infinite distance
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "intersect.h"
#include "grid-iter.h"

#include "far-light.h"

using namespace snogray;



// Generate around NUM samples of this light and add them to SAMPLES.
// Return the actual number of samples (NUM is only a suggestion).
//
unsigned
FarLight::gen_samples (const Intersect &isec, unsigned num,
		       IllumSampleVec &samples)
  const
{
  // First detect cases where the light isn't visible at all given the
  // ISEC's surface normal.
  //
  float cos_n_dir_angle = isec.cos_n (isec.normal_frame.to (frame.z));
  float n_dir_angle = acos (cos_n_dir_angle);
  float min_angle = n_dir_angle - angle / 2;

  if (min_angle >= 2 * PIf)
    return 0;

  GridIter grid_iter (num);

  float u, v;
  while (grid_iter.next (u, v))
    {
      // Sample onto the upper part of a cylinder.  The total height of
      // the cylinder is 2 (from -1 to 1), and the height of the sample
      // area correponds to ANGLE (so that if ANGLE is 4*PI, the entire
      // cylinder surface, from z -1 to 1, will be sampled).
      //
      float z = 1 - u * angle * 0.5f * INV_PIf;
      float r = sqrt (1 - z * z);
      float phi = v * 2 * PIf;
      float x = r * cos (phi), y = r * sin (phi);
      Vec s_dir = isec.normal_frame.to (frame.from (Vec (x, y, z)));

      if (isec.cos_n (s_dir) > 0)
	samples.push_back (IllumSample (s_dir, intensity, pdf, 0, this));
    }

  return grid_iter.num_samples ();
}



// For every sample from BEG_SAMPLE to END_SAMPLE which intersects this
// light, and where light is closer than the sample's previously recorded
// light distance (or the previous distance is zero), overwrite the
// sample's light-related fields with information from this light.
//
void
FarLight::filter_samples (const Intersect &isec, 
			  const IllumSampleVec::iterator &beg_sample,
			  const IllumSampleVec::iterator &end_sample)
  const
{
  Vec light_normal_dir = isec.normal_frame.to (frame.z);

  for (IllumSampleVec::iterator s = beg_sample; s != end_sample; ++s)
    if (s->dist == 0 && dot (s->dir, light_normal_dir) <= max_cos)
      {
	s->val = intensity;
	s->light_pdf = pdf;
	s->light = this;
      }
}


// arch-tag: 879b496d-2a8d-4a7e-8d0a-f92d67d4f165
