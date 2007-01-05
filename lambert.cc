// lambert.cc -- Lambertian reflectance function
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//
#include<iostream>

#include "intersect.h"
#include "cos-dist.h"
#include "grid-iter.h"

#include "lambert.h"

using namespace snogray;

// There's only one possible lambert object so create it here.
//
const Lambert *snogray::lambert = new Lambert;

// Generate (up to) NUM samples of this BRDF and add them to SAMPLES.
// For best results, they should be distributed according to the BRDF's
// importance function.
//
unsigned
Lambert::gen_samples (const Intersect &isec, unsigned num,
		      IllumSampleVec &samples)
  const
{
  CosDist dist;

  GridIter grid_iter (num);

  float u, v;
  while (grid_iter.next (u, v))
    {
      float pdf;
      Vec z_norm_dir = dist.sample (u, v, pdf);
      Vec s_dir = isec.z_normal_to_world (z_norm_dir);
      if (isec.cos_n (s_dir) > 0)
	samples.push_back (IllumSample (s_dir, isec.color * pdf, pdf));
    }

  return grid_iter.num_samples ();
}

// Add reflectance information for this BRDF to samples from BEG_SAMPLE
// to END_SAMPLE.
//
void
Lambert::filter_samples (const Intersect &isec, 
			 const IllumSampleVec::iterator &beg_sample,
			 const IllumSampleVec::iterator &end_sample)
  const
{
  CosDist dist;

  for (IllumSampleVec::iterator s = beg_sample; s != end_sample; ++s)
    if (! s->invalid)
      {
	s->brdf_pdf = dist.pdf (isec.cos_n (s->dir));
	s->refl = isec.color * s->brdf_pdf;
      }
}

// arch-tag: f61dbf3f-a5eb-4747-9bc5-18e793f35b6e
