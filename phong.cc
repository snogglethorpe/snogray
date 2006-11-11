// phog.cc -- Phong reflectance function
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <list>

#include "snogmath.h"
#include "intersect.h"
#include "phong-dist.h"
#include "cos-dist.h"
#include "grid-iter.h"

#include "phong.h"


using namespace Snogray;


// Source of "constant" (not-to-be-freed) Phong BRDFs
//
const Phong *
Snogray::phong (const Color &spec_col, float exp)
{
  static std::list<Phong> phongs;

  for (std::list<Phong>::iterator p = phongs.begin (); p != phongs.end (); p++)
    if (p->exponent == exp && p->specular_color == spec_col)
      return &(*p);

  phongs.push_front (Phong (spec_col, exp));

  return &phongs.front ();
}



// The details of phong evaluation are in this class.
//
struct PhongCalc
{
  PhongCalc (const Phong &_phong, const Intersect &_isec)
    : phong (_phong), isec (_isec),
      phong_dist (_phong.exponent), diff_dist (),
      diff_weight (isec.color.intensity ()),
      inv_diff_weight (diff_weight == 0 ? 0 : 1 / diff_weight),
      inv_spec_weight (diff_weight == 1 ? 0 : 1 / (1 - diff_weight))
  { }

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

    return isec.color * diff + phong.specular_color * spec;
  }

  void gen_sample (float u, float v, IllumSampleVec &samples)
  {
    Vec l, h;
    if (u < diff_weight)
      {
	float scaled_u = u * inv_diff_weight;
	l = isec.z_normal_to_world (diff_dist.sample (scaled_u, v));
	h = (isec.v + l).unit ();
      }
    else
      {
	float scaled_u = (u - diff_weight) * inv_spec_weight;
	h = isec.z_normal_to_world (phong_dist.sample (scaled_u, v));
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

  void filter_sample (const IllumSampleVec::iterator &s)
  {
    const Vec &l = s->dir;
    const Vec h = (isec.v + l).unit ();
    s->refl = val (l, h, s->brdf_pdf);
  }

  const Phong &phong;

  const Intersect &isec;

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



// Generate around NUM samples of this BRDF and add them to SAMPLES.
// NUM is only a suggestion.
//
unsigned
Phong::gen_samples (const Intersect &isec, unsigned num,
		    IllumSampleVec &samples)
  const
{
  PhongCalc calc (*this, isec);

  GridIter grid_iter (num);

  float u, v;
  while (grid_iter.next (u, v))
    calc.gen_sample (u, v, samples);

  return grid_iter.num_samples ();
}

// Add reflectance information for this BRDF to samples from BEG_SAMPLE
// to END_SAMPLE.
//
void
Phong::filter_samples (const Intersect &isec, 
		       const IllumSampleVec::iterator &beg_sample,
		       const IllumSampleVec::iterator &end_sample)
  const
{
  PhongCalc calc (*this, isec);

  for (IllumSampleVec::iterator s = beg_sample; s != end_sample; s++)
    if (! s->invalid)
      calc.filter_sample (s);
}


// arch-tag: 11e5304d-111f-4597-a164-f08bd49e1563
