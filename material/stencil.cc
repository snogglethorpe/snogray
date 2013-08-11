// stencil.cc -- Masking material for partial transparency/translucency
//
//  Copyright (C) 2010-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "intersect/intersect.h"
#include "media.h"
#include "fresnel.h"
#include "bsdf.h"
#include "util/compiler.h"

#include "stencil.h"

using namespace snogray;


namespace { // keep local to file

// Stencil BSDF representing the stencil at a particular intersection.
//
class StencilBsdf : public Bsdf
{
public:

  StencilBsdf (const Color &_opacity, Bsdf *_underlying_bsdf,
	       const Intersect &isec)
    : Bsdf (isec),
      opacity (_opacity),
      opacity_intens (_opacity.intensity ()),
      inv_opacity_intens (opacity_intens == 0 ? 0 : 1 / opacity_intens),
      underlying_bsdf (_underlying_bsdf)
  {
    // When nearly opaque or nearly transparent, use full
    // opacity/transparency instead, which can be substantially more
    // efficient.  Doing it here allows other code to not worry about
    // such details.
    //
    if (opacity_intens < 0.001f)
      opacity_intens = inv_opacity_intens = 0;
    else if (opacity_intens > 0.999f)
      opacity_intens = inv_opacity_intens = 1;
  }

  // Return a sample of this BSDF, based on the parameter PARAM.
  //
  virtual Sample sample (const UV &param, unsigned flags) const
  {
    bool thru_ok
      = ((opacity_intens < 1)
	 && ((flags & (TRANSMISSIVE|SPECULAR)) == (TRANSMISSIVE|SPECULAR)));
    bool undl_ok
      = ((opacity_intens > 0)
	 && underlying_bsdf
	 && underlying_bsdf->supports (flags));

    if (!thru_ok && !undl_ok)
      return Sample ();
    else if (! thru_ok)
      return underlying_bsdf->sample (param, flags);
    else if (! undl_ok || param.u > opacity_intens)
      {
	float cos_n = isec.cos_n (isec.v);
	if (unlikely (cos_n == 0))
	  return Sample ();
	else
	  return Sample ((1 - opacity) / cos_n,
			 undl_ok ? 1 - opacity_intens : 1,
			 -isec.v,
			 SPECULAR|TRANSMISSIVE|TRANSLUCENT);
      }
    else
      {
	UV scaled_param (param.u * inv_opacity_intens, param.v);
	Sample samp = underlying_bsdf->sample (scaled_param, flags);
	samp.val *= opacity;
	samp.pdf *= inv_opacity_intens;
	return samp;
      }
  }

  // Evaluate this BSDF in direction DIR, and return its value and pdf.
  // If FLAGS is specified, then only the given types of surface
  // interaction are considered.
  //
  virtual Value eval (const Vec &dir, unsigned flags) const
  {
    if (underlying_bsdf)
      {
	Value val = underlying_bsdf->eval (dir, flags);
	val.val *= opacity;
	val.pdf *= inv_opacity_intens;
	return val;
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
    unsigned flags = 0;
    if (opacity > 0)
      flags |= underlying_bsdf->supports (limit);
    if (opacity < 1)
      flags |= TRANSMISSIVE|SPECULAR;
    return flags;
  }

private:

  Color opacity;
  float opacity_intens, inv_opacity_intens;
  Bsdf *underlying_bsdf;
};

} // namespace


// Return a new BSDF object for this material instantiated at ISEC.
//
Bsdf *
Stencil::get_bsdf (const Intersect &isec) const
{
  Color opac = opacity.eval (isec);
  Bsdf *underlying_bsdf = underlying_material->get_bsdf (isec);
  
  // This is a common situation, so it's worth optimizing for it.
  //
  if (opac >= 1)
    return underlying_bsdf;

  return new (isec) StencilBsdf (opac, underlying_bsdf, isec);
}

// Return the transmittance of this material at the intersection
// described by ISEC_INFO in medium MEDIUM.
//
// Note that this method only applies to "simple"
// transparency/translucency, where transmitted rays don't change
// direction; materials that are conceptually "transparent," but which
// exhibit more complex effects like refraction (which change the
// direction) may return zero from this method.
//
Color
Stencil::transmittance (const Surface::IsecInfo &isec_info, const Medium &)
  const
{
  return max (1 - opacity.eval (isec_info.tex_coords ()), 0);
}
