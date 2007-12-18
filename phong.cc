// phong.cc -- Phong material
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

#include <list>

#include "snogmath.h"
#include "intersect.h"
#include "phong-dist.h"
#include "cos-dist.h"
#include "grid-iter.h"
#include "brdf.h"

#include "phong.h"


using namespace snogray;



// The details of phong evaluation are in this class.
//
class PhongBrdf : public Brdf
{
public:

  PhongBrdf (const Phong &_phong, const Intersect &_isec)
    : Brdf (_isec), phong (_phong),
      phong_dist (_phong.exponent), diff_dist (),
      diff_weight (phong.color.intensity ()),
      inv_diff_weight (diff_weight == 0 ? 0 : 1 / diff_weight),
      inv_spec_weight (diff_weight == 1 ? 0 : 1 / (1 - diff_weight))
  { }

  // Generate around NUM samples of this BRDF and add them to SAMPLES.
  // NUM is only a suggestion.
  //
  virtual unsigned gen_samples (unsigned num, IllumSampleVec &samples) const
  {
    GridIter grid_iter (num);

    float u, v;
    while (grid_iter.next (u, v))
      gen_sample (u, v, samples);

    return grid_iter.num_samples ();
  }

  // Add reflectance information for this BRDF to samples from BEG_SAMPLE
  // to END_SAMPLE.
  //
  virtual void filter_samples (const IllumSampleVec::iterator &beg_sample,
			       const IllumSampleVec::iterator &end_sample)
    const
  {
    for (IllumSampleVec::iterator s = beg_sample; s != end_sample; s++)
      if (! s->invalid)
	filter_sample (s);
  }

private:

  // Return the phong reflectance for the sample in direction L, where H is
  // the half-vector.  The pdf is returned in PDF.
  //
  Color val (const Vec &l, const Vec &h, float &pdf) const
  {
    float nh = isec.cos_n (h), nl = isec.cos_n (l);

    // Cosine of angle between view angle and half-way vector (also
    // between light-angle and half-way vector -- lh == vh).
    //
    float vh = isec.cos_v (h);

    // The division by 4 * VH when calculating the pdf here is intended
    // to compensate for the fact that the underlying distribution
    // PHONG_DIST is actually that of the half-vector H, whereas the pdf
    // we want should be the distribution of the light-vector L.  I don't
    // really understand why it works, but it's in the PBRT book, and
    // seems to have good results.
    //
    float spec = phong_dist.pdf (nh);
    float spec_pdf = spec / (4 * vh);

    float diff = diff_dist.pdf (nl);
    float diff_pdf = diff;	// identical

    pdf = diff_pdf * diff_weight + spec_pdf * (1 - diff_weight);

    return phong.color * diff + phong.specular_color * spec;
  }

  void gen_sample (float u, float v, IllumSampleVec &samples) const
  {
    Vec l, h;
    if (u < diff_weight)
      {
	float scaled_u = u * inv_diff_weight;
	l = diff_dist.sample (scaled_u, v);
	h = (isec.v + l).unit ();
      }
    else
      {
	float scaled_u = (u - diff_weight) * inv_spec_weight;
	h = phong_dist.sample (scaled_u, v);
	if (isec.cos_v (h) < 0)
	  h = -h;
	l = isec.v.mirror (h);
      }

    if (isec.cos_n (l) > 0)
      {
	float pdf;
	Color f = val (l, h, pdf);

	samples.push_back (IllumSample (l, f, pdf));
      }
  }

  void filter_sample (const IllumSampleVec::iterator &s) const
  {
    const Vec &l = s->dir;
    const Vec h = (isec.v + l).unit ();
    s->refl = val (l, h, s->brdf_pdf);
  }

  const Phong &phong;

  // Sample distributions for specular and diffuse components.
  //
  const PhongDist phong_dist;
  const CosDist diff_dist;

  // Weight used for sampling diffuse component (0 = don't sample
  // diffuse at all, 1 = only sample diffuse).  The "specular" component
  // has a weight of (1 - DIFF_WEIGHT).
  //
  float diff_weight;

  // 1 / DIFF_WEIGHT, and 1 / (1 - DIFF_WEIGHT).
  //
  float inv_diff_weight, inv_spec_weight;
};


// Make a BRDF object for this material instantiated at ISEC.
//
Brdf *
Phong::get_brdf (const Intersect &isec) const
{
  return new (isec) PhongBrdf (*this, isec);
}


// arch-tag: 11e5304d-111f-4597-a164-f08bd49e1563
