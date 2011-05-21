// thin-glass.cc -- ThinGlass (thin, transmissive, reflective) material
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

#include "intersect.h"
#include "media.h"
#include "fresnel.h"
#include "bsdf.h"

#include "thin-glass.h"

using namespace snogray;


// Common information used for refraction methods.
//
class ThinGlassBsdf : public Bsdf
{
public:

  ThinGlassBsdf (const ThinGlass &_thin_glass, const Intersect &isec)
    : Bsdf (isec), thin_glass (_thin_glass)
  { }

  // Return a sample of this BSDF, based on the parameter PARAM.
  //
  virtual Sample sample (const UV &param, unsigned flags) const
  {
    if (flags & SPECULAR)
      {
	// Clear all but the direction flags.  This means it will be
	// either REFLECTIVE, TRANSMISSIVE, or REFLECTIVE|TRANSMISSIVE.
	//
	flags &= ALL_DIRECTIONS;

	// Calculate fresnel surface reflection at the ray angle
	//
	float cos_xmit_angle = isec.cos_n (isec.v);
	float medium_ior = isec.media.medium.ior;
	float refl
	  = Fresnel (medium_ior, thin_glass.ior).reflectance (cos_xmit_angle);

	// Render transmitted light (some light is lost due to fresnel
	// reflection from the back surface).
	//
	Color xmit = thin_glass.color * (1 - refl);

	// If we're only allowed to choose a single direction, always
	// return that, otherwise choose between them based on their
	// relative strengths.
	//
	if (flags == TRANSMISSIVE || param.u < (xmit / (xmit + refl)))
	  // Transmitted sample.
	  return Sample (xmit, 1, -isec.v, SPECULAR|TRANSMISSIVE);
	else
	  // Reflected sample.
	  return Sample (refl, 1, isec.v.mirror (Vec (0, 0, 1)),
			 SPECULAR|REFLECTIVE);
      }

    return Sample ();
  }

  // Evaluate this BSDF in direction DIR, and return its value and pdf.
  // If FLAGS is specified, then only the given types of surface
  // interaction are considered.
  //
  virtual Value eval (const Vec &, unsigned) const
  {
    return Value ();		// we're specular, so all samples fail
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
      (limit & SPECULAR)
      ? (TRANSMISSIVE | REFLECTIVE | SPECULAR) & limit
      : 0;
  }

private:

  const ThinGlass &thin_glass;
};


// Return a new BSDF object for this material instantiated at ISEC.
//
Bsdf *
ThinGlass::get_bsdf (const Intersect &isec) const
{
  return new (isec) ThinGlassBsdf (*this, isec);
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
ThinGlass::transmittance (const Surface::IsecInfo &isec_info,
			  const Medium &medium)
  const
{
  Vec norm = isec_info.normal ();
  
  // Calculate fresnel surface reflection at the ray angle
  //
  float cos_xmit_angle = dot (norm, -isec_info.ray.dir);
  float refl = Fresnel (medium.ior, ior).reflectance (cos_xmit_angle);

  // Transmitted light (some light is lost due to fresnel reflection
  // from the back surface).
  //
  Color xmit = color * (1 - refl);

  return xmit;
}


// arch-tag: cd843fe9-2c15-4212-80d7-7e302850c1a7
