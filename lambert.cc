// lambert.cc -- Lambertian material
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <list>

#include "intersect.h"
#include "cos-dist.h"
#include "grid-iter.h"
#include "brdf.h"

#include "lambert.h"

using namespace snogray;




// The details of lambertian evaluation are in this class.
//
class LambertBrdf : public Brdf
{
public:

  LambertBrdf (const Lambert &_lambert, const Intersect &_isec)
    : Brdf (_isec), color (_lambert.color.eval (_isec))
  { }

  // Generate around NUM samples of this BRDF and add them to SAMPLES.
  // Return the actual number of samples (NUM is only a suggestion).
  //
  virtual unsigned gen_samples (unsigned num, IllumSampleVec &samples) const
  {
    GridIter grid_iter (num);

    float u, v;
    while (grid_iter.next (u, v))
      {
	float pdf;
	Vec dir = dist.sample (u, v, pdf);
	if (isec.cos_n (dir) > 0 && isec.cos_geom_n (dir) > 0)
	  samples.push_back (IllumSample (dir, color * INV_PIf, pdf,
					  IllumSample::REFLECTIVE
					  |IllumSample::DIFFUSE));
      }

    return grid_iter.num_samples ();
  }

  // Add reflectance information for this BRDF to samples from BEG_SAMPLE
  // to END_SAMPLE.
  //
  virtual void filter_samples (const IllumSampleVec::iterator &beg_sample,
			       const IllumSampleVec::iterator &end_sample)
    const
  {
    for (IllumSampleVec::iterator s = beg_sample; s != end_sample; ++s)
      {
	s->brdf_val = color * INV_PIf;
	s->brdf_pdf = dist.pdf (isec.cos_n (s->dir));
	s->flags |= IllumSample::REFLECTIVE|IllumSample::DIFFUSE;
      }
  }

private:

  Color color;

  CosDist dist;
};


// Make a BRDF object for this material instantiated at ISEC.
//
Brdf *
Lambert::get_brdf (const Intersect &isec) const
{
  return new (isec) LambertBrdf (*this, isec);
}


// arch-tag: f61dbf3f-a5eb-4747-9bc5-18e793f35b6e
