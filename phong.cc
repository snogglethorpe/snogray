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



// Generate around NUM samples of this BRDF and add them to SAMPLES.
// NUM is only a suggestion.
//
unsigned
Phong::gen_samples (const Intersect &isec, unsigned num,
		    IllumSampleVec &samples)
  const
{
  PhongDist phong_dist (exponent);
  CosDist diff_dist;

  GridIter grid_iter (num);

  float u, v;
  while (grid_iter.next (u, v))
    {
      Vec l, h;
      if (u < 0.5)
	{
	  l = isec.z_normal_to_world (diff_dist.sample (u * 2, v));
	  h = (isec.v + l).unit ();
	}
      else
	{
	  h = isec.z_normal_to_world (phong_dist.sample (u * 2 - 1, v));
	  if (isec.cos_v (h) < 0)
	    h = -h;
	  l = isec.v.mirror (h);
	}

      float nl = isec.cos_n (l);
      float nh = isec.cos_n (h);
      float vh = isec.cos_v (h); // == h . l

      if (nl < 0)
	continue;

      // The division by 4 * VH when calculating the pdf here is intended to
      // compensate for the fact that the underlying distribution PHONG_DIST
      // is actually that of the half-vector H, whereas the pdf we want
      // should be the distribution of the light-vector L.  I don't really
      // understand why it works, but it's in the PBRT book, and seems to
      // have good results.
      //
      float spec = phong_dist.pdf (nh);
      float spec_pdf = spec / (4 * vh);

      float diff = diff_dist.pdf (nl);
      float diff_pdf = diff;	// identical

      Color f = isec.color * diff + specular_color * spec;
      float pdf = 0.5f * (spec_pdf + diff_pdf);

      samples.push_back (IllumSample (l, f, pdf));
    }

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
  PhongDist phong_dist (exponent);
  CosDist diff_dist;

  for (IllumSampleVec::iterator s = beg_sample; s != end_sample; ++s)
    if (! s->invalid)
      {
	const Vec &l = s->dir;
	const Vec h = (isec.v + l).unit ();

	float nl = isec.cos_n (l);
	float nh = isec.cos_n (h);
	float vh = isec.cos_v (h); // == h . l

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

	s->refl = isec.color * diff + specular_color * spec;
	s->brdf_pdf = 0.5f * (spec_pdf + diff_pdf);
      }
}


// arch-tag: 11e5304d-111f-4597-a164-f08bd49e1563
