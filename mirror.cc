// mirror.cc -- Mirror (reflective) material
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
#include "lambert.h"
#include "brdf.h"

#include "mirror.h"


using namespace snogray;


// A mirror with a simple lambertian underlying material.
//
Mirror::Mirror (const Ior &_ior,
		const TexVal<Color> &_reflectance, const TexVal<Color> &col)
  : ior (_ior), reflectance (_reflectance),
    underlying_material ((!col.tex && col.default_val < Eps)
			 ? 0 : new Lambert (col))
{ }


class MirrorBrdf : public Brdf
{
public:

  MirrorBrdf (const Mirror &_mirror, const Intersect &_isec)
    : Brdf (_isec),
      underlying_brdf (_mirror.underlying_material
		       ? _mirror.underlying_material->get_brdf (_isec)
		       : 0),
      fres (isec.media.medium.ior, _mirror.ior),
      reflectance (_mirror.reflectance.eval (isec))
  { }

  // Return a sample of this BRDF, based on the parameter PARAM.
  //
  virtual Sample sample (const UV &param, unsigned flags) const
  {
    if ((flags & (SPECULAR|REFLECTIVE)) == (SPECULAR|REFLECTIVE))
      {
	// The cosine of the angle between the reflected ray and the
	// surface normal.  For reflection this angle is the same as the
	// angle between the view ray and the normal.
	//
	float cos_refl_angle = isec.cos_n (isec.v);

	if (cos_refl_angle != 0)
	  {
	    // Generate specular sample.
	    //
	    Color refl
	      = (reflectance
		 * fres.reflectance (cos_refl_angle) / cos_refl_angle);

	    if (refl > Eps && isec.cos_geom_n (isec.v) > 0 /* XXX ?? XXX ??  */)
	      return Sample (refl, 1, isec.v.mirror (Vec (0, 0, 1)), 
			     SPECULAR|REFLECTIVE);
	    else if (underlying_brdf)
	      {
		// We have an underlying BRDF, so generate a sample from that.
		//
		Sample samp = underlying_brdf->sample (param, flags);

		// Tweak the result.
		//
		samp.val *= remove_specular_reflection (samp.dir);

		return samp;
	      }
	  }
      }

    return Sample ();
  }

  // Evaluate this BRDF in direction DIR, and return its value and pdf.
  //
  virtual Value eval (const Vec &) const
  {
    return Value ();		// we're specular, so all samples fail
  }

private:

  // Return an adjust factor to remove any light reflected by perfect
  // specular reflection.
  //
  Color remove_specular_reflection (const Vec &dir) const
  {
    float fres_refl = fres.reflectance (isec.cos_n (dir));
    const Color refl = fres_refl * reflectance;
    return 1 - refl;
  }

  const Brdf *underlying_brdf;

  const Fresnel fres;

  Color reflectance;
};


// Make a BRDF object for this material instantiated at ISEC.
//
Brdf *
Mirror::get_brdf (const Intersect &isec) const
{
  return new (isec) MirrorBrdf (*this, isec);
}


// arch-tag: b895139d-fe9f-414a-9665-3b5e4b8f691a
