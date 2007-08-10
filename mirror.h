// mirror.h -- Mirror (perfectly reflective) material
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
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

namespace snogray {

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

  // Generate around NUM samples of this BRDF and add them to SAMPLES.
  // Return the actual number of samples (NUM is only a suggestion).
  //
  virtual unsigned gen_samples (const Intersect &isec, unsigned num,
				IllumSampleVec &samples)
    const;

  // Add reflectance information for this BRDF to samples from BEG_SAMPLE
  // to END_SAMPLE.
  //
  virtual void filter_samples (const Intersect &isec, 
			       const IllumSampleVec::iterator &beg_sample,
			       const IllumSampleVec::iterator &end_sample)
    const;

  // Remove from SAMPLES any light reflected by perfect specular reflection.
  //
  void remove_specular_reflection (const Intersect &isec,
				   IllumSampleVec::iterator beg_sample,
				   IllumSampleVec::iterator end_sample)
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
  Mirror (const Ior &_ior, const Color &_reflectance, const Color &col = 0)
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
