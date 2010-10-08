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

#include "snogmath.h"
#include "vec.h"
#include "media.h"
#include "intersect.h"
#include "ward-dist.h"
#include "cos-dist.h"
#include "bsdf.h"

#include "cook-torrance.h"


using namespace snogray;



// The details of cook-torrance evaluation are in this class.
//
class CookTorranceBsdf : public Bsdf
{
public:

  // Values of M (RMS slope) less than this are considered "glossy".
  //
  // This should be a simple named constant, but C++ (stupidly)
  // disallows non-integral named constants.  Someday when "constexpr"
  // support is widespread, that can be used instead.
  //
  static float glossy_m () { return 0.5; }

  CookTorranceBsdf (const CookTorrance &ct, const Intersect &_isec)
    : Bsdf (_isec),
      m (ct.m.eval (isec)), gloss_dist (m), diff_dist (),
      diff_col (ct.color.eval (isec)),
      gloss_col (ct.gloss_color.eval (isec)),
      diff_intens (diff_col.intensity ()),
      gloss_intens (gloss_col.intensity ()),
      diff_weight ((diff_intens + gloss_intens) == 0
		   ? 0
		   : diff_intens / (diff_intens + gloss_intens)),
      inv_diff_weight (diff_weight == 0 ? 0 : 1 / diff_weight),
      inv_gloss_weight (diff_weight == 1 ? 0 : 1 / (1 - diff_weight)),
      fres (isec.media.medium.ior, ct.ior),
      nv (isec.cos_n (isec.v)),
      inv_4_nv ((nv != 0) ? 1 / (4 * nv) : 0),
      gloss_layer (m < glossy_m() ? GLOSSY : DIFFUSE),
      have_layers ((diff_weight > 0 ? DIFFUSE : 0)
		   | (diff_weight < 1 ? gloss_layer : 0))
  { }

  // Return a sample of this BSDF, based on the parameter PARAM.
  //
  virtual Sample sample (const UV &param, unsigned desired = ALL) const
  {
    Vec l, h;
    unsigned flags = REFLECTIVE;
    float u = param.u, v = param.v;

    if (! (desired & REFLECTIVE))
      goto fail;

    // Remove all flags except those BSDF layers we can support.
    //
    desired &= have_layers;

    if (! desired)
      goto fail;

    // Handle flipped eye vector...
    // XXX is this the right thing...?  What should we do here?
    //
    if (isec.v.z < 0)
      goto fail;

    // DESIRED_DIFF_WEIGHT is the probability we choose the diffuse
    // layer.  If DESIRED contains both layers, then DESIRED_DIFF_WEIGHT
    // == CookTorranceBsdf::diff_weight; if DESIRED only includes the
    // diffuse layer, it will be 1, and otherwise it will be 0.
    //
    // Similarly, INV_DESIRED_DIFF_WEIGHT and INV_DESIRED_GLOSS_WEIGHT are
    // local versions of INV_DIFF_WEIGHT and INV_GLOSS_WEIGHT, and have
    // values of 1 / DESIRED_DIFF_WEIGHT and 1 / (1 - DESIRED_DIFF_WEIGHT)
    // respectively, with care taken to avoid dividing by zero; the
    // structure of the code below ensures that they won't actually be
    // used in the case where divide-by-zero would have occured.
    //
    float desired_diff_weight;
    float inv_desired_diff_weight, inv_desired_gloss_weight;

    if (desired == (DIFFUSE|gloss_layer))
      {
	// Both layers desired, so the DESIRED_ values are the same as
	// the global ones.

	desired_diff_weight = diff_weight;
	inv_desired_diff_weight = inv_diff_weight;
	inv_desired_gloss_weight = inv_gloss_weight;
      }
    else if (desired == DIFFUSE)
      {
	// Only diffuse layer desired.

	desired_diff_weight = 1;
	inv_desired_diff_weight = 1;
	inv_desired_gloss_weight = 0; // not actually used
      }
    else
      {
	// Only glossy layer desired.

	desired_diff_weight = 0;
	inv_desired_diff_weight = 0; // not actually used
	inv_desired_gloss_weight = 1;
      }

    if (u < desired_diff_weight)
      {
	// Adjust U so that the diffuse range (0 - DESIRED_DIFF_WEIGHT)
	// is mapped to 0 - 1.
	//
	if (desired != DIFFUSE)
	  u = u * inv_desired_diff_weight;

	l = diff_dist.sample (UV (u, v));
	h = (isec.v + l).unit ();
	flags |= DIFFUSE;
      }
    else
      {
	// Adjust U so that the glossy range (DESIRED_DIFF_WEIGHT - 1) is
	// mapped to 0 - 1.
	//
	u = (u - desired_diff_weight) * inv_desired_gloss_weight;

	h = gloss_dist.sample (UV (u, v));
	if (isec.cos_v (h) < 0)
	  h = -h;
	l = isec.v.mirror (h);
	flags |= gloss_layer;
      }

    if (isec.cos_n (l) > Eps && isec.cos_geom_n (l) > Eps)
      {
	float pdf;
	Color f = val (l, h, desired, desired_diff_weight, pdf);
	return Sample (f, pdf, l, flags);
      }

  fail:
    return Sample ();
  }

  // Evaluate this BSDF in direction DIR, and return its value and pdf.
  // If FLAGS is specified, then only the given types of surface
  // interaction are considered.
  //
  virtual Value eval (const Vec &dir, unsigned flags) const
  {
    float cos_n = isec.cos_n (dir);
    if ((flags & REFLECTIVE) && cos_n > 0)
      {
	// Remove all flags except those BSDF layers we can support.
	//
	flags &= have_layers;

	// DESIRED_DIFF_WEIGHT is the probability we choose the
	// diffuse layer.  If FLAGS contains both layers, then
	// DESIRED_DIFF_WEIGHT == CookTorranceBsdf::diff_weight; if
	// FLAGS only includes the diffuse layer, it will be 1, and
	// otherwise it will be 0.
	//
	float desired_diff_weight
	  = ((flags == (DIFFUSE|gloss_layer))
	     ? diff_weight
	     : (flags == DIFFUSE)
	     ? 1
	     : 0);

	const Vec h = (isec.v + dir).unit ();

	float pdf;
	Color f = val (dir, h, flags, desired_diff_weight, pdf);

	return Value (f, pdf);
      }

    return Value ();
  }

  // Return a bitmask of flags from Bsdf::Flags, describing what
  // types of scatting this BSDF supports.  The returned value will
  // include only flags in LIMIT (default, all flags).
  //
  // The various fields (Bsdf::ALL_LAYERS, Bsdf::ALL_DIRECTIONS) in
  // the returned value should be consistent -- a layer flag like
  // Bsdf::DIFFUSE should only be included if that layer is
  // supported by one of the sample-directions
  // (e.g. Bsdf::REFLECTIVE) in the return value, and vice-versa.
  //
  virtual unsigned supports (unsigned limit) const
  {
    unsigned refl_flags
      = ((diff_weight > 0 ? DIFFUSE : 0)
	 | (diff_weight < 1 ? gloss_layer : 0));
    return
      ((limit & REFLECTIVE) && (limit & refl_flags))
      ? ((REFLECTIVE | refl_flags) & limit)
      : 0;
  }

private:

  // Calculate D (microfacet distribution) term.  Traditionally
  // Cook-torrance uses a Beckmann distribution for this, but we use the
  // Ward Isotropic distribution becuase it's easy to sample.
  //
  float D (float nh) const
  {
    return gloss_dist.pdf (nh);
  }
  float D_pdf (float nh, float vh) const
  {
    // The division by 4 * VH here is intended to compensate for the fact
    // that the underlying distribution GLOSS_DIST is actually that of the
    // half-vector H, whereas the pdf we want should be the distribution of
    // the light-vector L.  I don't really understand why it works, but
    // it's in the PBRT book, and seems to have good results.
    //
    return gloss_dist.pdf (nh) / (4 * vh);
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

  // Return the CT reflectance for the sample in direction L, where H
  // is the half-vector.  The pdf is returned in PDF.  FLAGS controls
  // which layers are used in the evaluation.  DESIRED_DIFF_WEIGHT is
  // the probability of choosing the diffuse layer (which may be
  // different than CookTorranceBsdf::diff_weight in the case where
  // the user specified a restricted set of layers), and is used to
  // calculate the PDF.
  //
  Color val (const Vec &l, const Vec &h,
	     unsigned flags, float desired_diff_weight,
	     float &pdf)
    const
  {
    float nl = isec.cos_n (l);

    Color col = 0;
    pdf = 0;

    if (flags & DIFFUSE)
      {
	// Diffuse term is a simple lambertian (cosine) distribution, and
	// its pdf is constant.
	//
	float diff = INV_PIf;
	float diff_pdf = diff_dist.pdf (nl);

	pdf += diff_pdf * desired_diff_weight;
	col += diff_col * diff;
      }

    if (flags & gloss_layer)
      {
	float nh = isec.cos_n (h);

	// Avoid divide-by-zero if NL == 0.  Not a good situation, but I'm
	// not sure what else to do...
	//
	float inv_nl = (nl != 0) ? (1 / nl) : 0; 

	// Angle between view angle and half-way vector (also between
	// light-angle and half-way vector -- lh == vh).
	//
	float vh = isec.cos_v (h);

	// The Cook-Torrance glossy-lobe term is:
	//
	//    f_s = F * D * G / (4 * (N dot V) * (N dot L))
	//
	// We sample the glossy-lobe using the D component only, so the pdf
	// is only based on that.
	//
	float gloss = F (vh) * D (nh) * G (vh, nh, nl) * inv_4_nv * inv_nl;
	float gloss_pdf = D_pdf (nh, vh);

	pdf += gloss_pdf * (1 - desired_diff_weight);
	col += gloss_col * gloss;
      }

    return col;
  }

  // M value used.
  //
  float m;

  // Sample distributions for glossy and diffuse components.
  //
  const WardDist gloss_dist;
  const CosDist diff_dist;

  // Color of diffuse/glossy components.
  //
  Color diff_col, gloss_col;

  // Intensity (sum of all color components) of diffuse and glossy
  // components.
  //
  float diff_intens, gloss_intens;

  // Weight used for sampling diffuse component (0 = don't sample
  // diffuse at all, 1 = only sample diffuse).  The glossy component
  // has a weight of (1 - DIFF_WEIGHT).
  //
  float diff_weight;

  // 1 / DIFF_WEIGHT, and 1 / (1 - DIFF_WEIGHT).
  //
  float inv_diff_weight, inv_gloss_weight;

  // Info for calculating the Fresnel term.
  //
  const Fresnel fres;

  // N dot V, which is the cosine of the angle between the eye ray (V),
  // and the surface normal (N), and 1 / (4 * (N dot V)).
  //
  float nv, inv_4_nv;

  // Bsdf layer flags to use for glossy samples.
  //
  unsigned gloss_layer;

  // The set of Bsdf layer flags for surface types we support; some
  // combination of DIFFUSE and GLOSSY.
  //
  unsigned have_layers;
};


// Return a new BSDF object for this material instantiated at ISEC.
//
Bsdf *
CookTorrance::get_bsdf (const Intersect &isec) const
{
  return new (isec) CookTorranceBsdf (*this, isec);
}


// arch-tag: a0a0049e-9af6-4438-ab58-081630151122
