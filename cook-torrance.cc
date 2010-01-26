// cook-torrance.cc -- Cook-Torrance material
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

#include "snogmath.h"
#include "vec.h"
#include "trace.h"
#include "intersect.h"
#include "ward-dist.h"
#include "cos-dist.h"
#include "grid-iter.h"
#include "brdf.h"

#include "cook-torrance.h"


using namespace snogray;



// The details of cook-torrance evaluation are in this class.
//
class CookTorranceBrdf : public Brdf
{
public:

  // Values of M (RMS slope) less than this are considered "glossy".
  //
  static const float GLOSSY_M = 0.5;

  CookTorranceBrdf (const CookTorrance &ct, const Intersect &_isec)
    : Brdf (_isec),
      m (ct.m.eval (isec)), spec_dist (m), diff_dist (),
      diff_col (ct.color.eval (isec)),
      spec_col (ct.specular_color.eval (isec)),
      diff_intens (diff_col.intensity ()),
      spec_intens (spec_col.intensity ()),
      diff_weight (diff_intens / (diff_intens + spec_intens)),
      inv_diff_weight (diff_weight == 0 ? 0 : 1 / diff_weight),
      inv_spec_weight (diff_weight == 1 ? 0 : 1 / (1 - diff_weight)),
      fres (isec.trace.medium.ior, ct.ior),
      nv (isec.cos_n (isec.v)),
      inv_4_nv ((nv != 0) ? 1 / (4 * nv) : 0),
      spec_flags (m < GLOSSY_M ? GLOSSY : DIFFUSE),
      have_surface_flags ((diff_weight > 0 ? DIFFUSE : 0)
			  | (diff_weight < 1 ? spec_flags : 0))
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
	Sample samp = sample (UV (u, v));
	if (samp.val > 0)
	  // XXX note we rely on the values of Brdf::Flags being the same as
	  // the correponding values in IllumSample::Flags!!
	  samples.push_back (IllumSample (samp.dir, samp.val, samp.pdf, samp.flags));
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
    for (IllumSampleVec::iterator s = beg_sample; s != end_sample; s++)
      filter_sample (s);
  }

  // Return a sample of this BRDF, based on the parameter PARAM.
  //
  virtual Sample sample (const UV &param, unsigned desired_flags = ALL) const
  {
    Vec l, h;
    unsigned flags = REFLECTIVE;
    float u = param.u, v = param.v;

    if (! (desired_flags & REFLECTIVE))
      goto fail;

    // Remove all flags except those reflecting surface types we can
    // support.
    //
    desired_flags &= have_surface_flags;

    if (! desired_flags)
      goto fail;

    if (desired_flags == DIFFUSE || u < diff_weight)
      {
	// If we chose between DIFFUSE and GLOSSY based on U, adjust U so
	// that the diffuse range (0 - DIFF_WEIGHT) is mapped to 0 - 1.
	//
	if (desired_flags != DIFFUSE)
	  u = u * inv_diff_weight;

	l = diff_dist.sample (u, v);
	h = (isec.v + l).unit ();
	flags |= DIFFUSE;
      }
    else
      {
	// If we chose between DIFFUSE and GLOSSY based on U, adjust U so
	// that the glossy range (DIFF_WEIGHT - 1) is mapped to 0 - 1.
	//
	if (desired_flags != DIFFUSE)
	  u = (u - diff_weight) * inv_spec_weight;

	h = spec_dist.sample (u, v);
	if (isec.cos_v (h) < 0)
	  h = -h;
	l = isec.v.mirror (h);
	flags |= spec_flags;
      }

    if (isec.cos_n (l) > Eps && isec.cos_geom_n (l) > Eps)
      {
	float pdf;
	Color f = val (l, h, pdf);
	return Sample (f, pdf, l, flags);
      }

  fail:
    return Sample ();
  }

  // Evaluate this BRDF in direction DIR, and return its value and pdf.
  //
  virtual Value eval (const Vec &dir) const
  {
    const Vec h = (isec.v + dir).unit ();
    float pdf;
    Color f = val (dir, h, pdf);
    return Value (f, pdf);
  }

private:

  // Calculate D (microfacet distribution) term.  Traditionally
  // Cook-torrance uses a Beckmann distribution for this, but we use the
  // Ward Isotropic distribution becuase it's easy to sample.
  //
  float D (float nh) const
  {
    return spec_dist.pdf (nh);
  }
  float D_pdf (float nh, float vh) const
  {
    // The division by 4 * VH here is intended to compensate for the fact
    // that the underlying distribution SPEC_DIST is actually that of the
    // half-vector H, whereas the pdf we want should be the distribution of
    // the light-vector L.  I don't really understand why it works, but
    // it's in the PBRT book, and seems to have good results.
    //
    return spec_dist.pdf (nh) / (4 * vh);
  }

  // Calculate F (fresnel) term
  //
  float F (float vh) const { return fres.reflectance (vh); }

  // Calculate G (microfacet masking/shadowing) term
  //
  //    G = min (1,
  //             2 * (N dot H) * (N dot V) / (V dot H),
  //             2 * (N dot H) * (N dot L) / (V dot H))
  //
  float G (float vh, float nh, float nl) const
  {
    return min (2 * nh * ((nv > nl) ? nl : nv) / vh, 1.f);
  }

  // Return the CT reflectance for the sample in direction L, where H is
  // the half-vector.  The pdf is returned in PDF.
  //
  Color val (const Vec &l, const Vec &h, float &pdf) const
  {
    float nh = isec.cos_n (h), nl = isec.cos_n (l);

    // Avoid divide-by-zero if NL == 0.  Not a good situation, but I'm
    // not sure what else to do...
    //
    float inv_nl = (nl != 0) ? (1 / nl) : 0; 

    // Angle between view angle and half-way vector (also between
    // light-angle and half-way vector -- lh == vh).
    //
    float vh = isec.cos_v (h);

    // The Cook-Torrance specular term is:
    //
    //    f_s = F * D * G / (4 * (N dot V) * (N dot L))
    //
    // We sample the specular term using the D component only, so the pdf
    // is only based on that.
    //
    float spec = F (vh) * D (nh) * G (vh, nh, nl) * inv_4_nv * inv_nl;
    float spec_pdf = D_pdf (nh, vh);

    // Diffuse term is a simple lambertian (cosine) distribution, and
    // its pdf is constant.
    //
    float diff = INV_PIf;
    float diff_pdf = diff_dist.pdf (nl);

    pdf = diff_pdf * diff_weight + spec_pdf * (1 - diff_weight);

    return diff_col * diff + spec_col * spec;
  }

  void filter_sample (const IllumSampleVec::iterator &s) const
  {
    const Vec &l = s->dir;
    const Vec h = (isec.v + l).unit ();
    s->brdf_val = val (l, h, s->brdf_pdf);
    s->flags |= IllumSample::REFLECTIVE;
  }

  // M value used.
  //
  float m;

  // Sample distributions for specular and diffuse components.
  //
  const WardDist spec_dist;
  const CosDist diff_dist;

  // Color of diffuse/specular components.
  //
  Color diff_col, spec_col;

  // Intensity (sum of all color components) of diffuse and specular
  // components.
  //
  float diff_intens, spec_intens;

  // Weight used for sampling diffuse component (0 = don't sample
  // diffuse at all, 1 = only sample diffuse).  The "specular" component
  // has a weight of (1 - DIFF_WEIGHT).
  //
  float diff_weight;

  // 1 / DIFF_WEIGHT, and 1 / (1 - DIFF_WEIGHT).
  //
  float inv_diff_weight, inv_spec_weight;

  // Info for calculating the Fresnel term.
  //
  const Fresnel fres;

  // N dot V, which is the cosine of the angle between the eye ray (V),
  // and the surface normal (N), and 1 / (4 * (N dot V)).
  //
  float nv, inv_4_nv;

  // Brdf flags to use for "specular" samples.
  //
  unsigned spec_flags;

  // The set of Brdf flags for surface types we support; some combination
  // of DIFFUSE and GLOSSY.
  //
  unsigned have_surface_flags;
};


// Return a new BRDF object for this material instantiated at ISEC.
//
Brdf *
CookTorrance::get_brdf (const Intersect &isec) const
{
  return new (isec) CookTorranceBrdf (*this, isec);
}


// arch-tag: a0a0049e-9af6-4438-ab58-081630151122
