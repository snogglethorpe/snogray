// mirror.h -- Mirror (perfectly reflective) material
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MIRROR_H__
#define __MIRROR_H__

#include "material.h"
#include "fresnel.h"

namespace Snogray {

// This is a BRDF used to eliminate specular reflection before calling a
// Mirror object's underlying "undercoat" BRDF.
//
class MirrorCoating : public Brdf
{
public:

  MirrorCoating (const Ior &_ior, const Color &_reflectance,
		 const Brdf *_underlying_brdf = 0)
    : ior (_ior), reflectance (_reflectance),
      underlying_brdf (_underlying_brdf)
  { }

  // Generate (up to) NUM samples of this BRDF and add them to SAMPLES.
  // For best results, they should be distributed according to the BRDF's
  // importance function.
  //
  virtual void gen_samples (const Intersect &isec, SampleRayVec &samples)
    const;

  // Modify the value of each of the light-samples in SAMPLES according to
  // the BRDF's reflectivity in the sample's direction.
  //
  virtual void filter_samples (const Intersect &isec, SampleRayVec &samples,
			       SampleRayVec::iterator from,
			       SampleRayVec::iterator to)
    const;

  // Remove from SAMPLES any light reflected by perfect specular reflection.
  //
  void remove_specular_reflection (const Intersect &isec,
				   SampleRayVec &samples,
				   SampleRayVec::iterator from,
				   SampleRayVec::iterator to)
    const;

  // Index of refraction for calculating fresnel reflection term.
  //
  Ior ior;

  // Amount of light reflected (further modulated by a fresnel reflection
  // term using IOR).
  //
  Color reflectance;

  // BRDF underlying the mirror coating, which does the real work.
  // This may be zero, for perfectly black surfaces.
  //
  const Brdf *underlying_brdf;
};

// A Material implementing perfect specular reflection.  It is structured
// as a specularly reflecting layer on top of an arbitrary BRDF to handle
// any light that gets throught the reflecting layer.  The reflecting layer
// may be a dielectic (like glass) with a non-complex index of refraction,
// in which case it will only reflect at some angles, or a conductor (whose
// index of refraction will also have an an extinction coefficient k),
// which will reflect at all angles.
//
class Mirror : public Material
{
public:

  Mirror (const Ior &_ior, const Color &_reflectance,
	  const Color &col, const Brdf *underlying_brdf)
    : Material (col, &mirror_coating),
      mirror_coating (_ior, _reflectance, underlying_brdf)
  { }
  Mirror (const Ior &_ior, const Color &_reflectance,
	  const Color &col = Color::black)
    : Material (col, &mirror_coating),
      mirror_coating (_ior, _reflectance, col < Eps ? 0 : lambert)
  { }

  virtual Color render (const Intersect &isec) const;

  // This is the MirrorCoating BRDF used to filter the underlying BRDF.
  //
  MirrorCoating mirror_coating;
};

}

#endif /* __MIRROR_H__ */

// arch-tag: b622d70c-03ff-49ee-a020-2a44ccfcfdb1
