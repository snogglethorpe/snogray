// phong.cc -- Phong material
//
//  Copyright (C) 2005-2010, 2012, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "util/snogmath.h"
#include "intersect/intersect.h"
#include "phong-dist.h"
#include "cos-dist.h"
#include "bsdf.h"

#include "phong.h"


using namespace snogray;



namespace { // keep local to file

// The details of phong evaluation are in this class.
//
class PhongBsdf : public Bsdf
{
public:

  PhongBsdf (const Phong &_phong, const Intersect &_isec)
    : Bsdf (_isec), phong (_phong),
      phong_dist (_phong.exponent), diff_dist (),
      diff_weight (phong.color.intensity ()),
      inv_diff_weight (diff_weight == 0 ? 0 : 1 / diff_weight),
      inv_spec_weight (diff_weight == 1 ? 0 : 1 / (1 - diff_weight))
  { }

private:

  // Return the phong reflectance for the sample in direction L, where H
  // is the half-vector.  The pdf is returned in PDF.  FLAGS controls
  // which layers are used in the evaluation.
  //
  Color val (const Vec &l, const Vec &h, float &pdf, unsigned flags) const
  {
    float nl = isec.cos_n (l);

    Color col = 0;
    pdf = 0;

    if (flags & DIFFUSE)
      {
	float diff = INV_PIf;
	float diff_pdf = diff_dist.pdf (nl);

	pdf += diff_pdf * diff_weight;
	col += phong.color * diff;
      }

    if (flags & GLOSSY)
      {
	float nh = isec.cos_n (h);

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

	pdf += spec_pdf * (1 - diff_weight);
	col += phong.specular_color * spec;
      }

    return col;
  }

  // Return a sample of this BSDF, based on the parameter PARAM.
  //
  virtual Sample sample (const UV &param, unsigned desired_flags = ALL) const
  {
    Vec l, h;
    unsigned flags = REFLECTIVE;
    float u = param.u, v = param.v;

    if (u < diff_weight)
      {
	float scaled_u = u * inv_diff_weight;
	l = diff_dist.sample (UV (scaled_u, v));
	h = (isec.v + l).unit ();
      }
    else
      {
	float scaled_u = (u - diff_weight) * inv_spec_weight;
	h = phong_dist.sample (UV (scaled_u, v));
	if (isec.cos_v (h) < 0)
	  h = -h;
	l = isec.v.mirror (h);
      }


    if (isec.cos_n (l) > Epsf && isec.cos_geom_n (l) > Epsf)
      {
	float pdf;
	Color f = val (l, h, pdf, desired_flags);
	return Sample (f, pdf, l, flags);
      }

    fail:
      return Sample (0, 0, l, flags);
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
	const Vec h = (isec.v + dir).unit ();
	float pdf;
	Color f = val (dir, h, pdf, flags);
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
    return
      ((limit & REFLECTIVE) && (limit & (DIFFUSE | GLOSSY)))
      ? ((REFLECTIVE | DIFFUSE | GLOSSY) & limit)
      : 0;
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

} // namespace


// Make a BSDF object for this material instantiated at ISEC.
//
Bsdf *
Phong::get_bsdf (const Intersect &isec) const
{
  return new (isec) PhongBsdf (*this, isec);
}


// arch-tag: 11e5304d-111f-4597-a164-f08bd49e1563
