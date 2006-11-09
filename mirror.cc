// mirror.cc -- Mirror (reflective) material
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "snogmath.h"
#include "mirror.h"

#include "intersect.h"
#include "scene.h"

using namespace Snogray;

Color
Mirror::render (const Intersect &isec) const
{
  float cos_refl_angle = isec.nv;
  float medium_ior = isec.trace.medium ? isec.trace.medium->ior : 1;
  const Fresnel fres (medium_ior, mirror_coating.ior);
  float fres_refl = fres.reflectance (cos_refl_angle);
  const Color refl = fres_refl * mirror_coating.reflectance;

  Color radiance;

  // First see if anything will be specularly reflected at this angle, and
  // if so, cast a reflection ray to find out what it is.
  //
  if (refl > Eps)
    {
      Vec mirror_dir = isec.v.mirror (isec.n);
      Ray mirror_ray (isec.pos, mirror_dir);
      Trace &sub_trace = isec.subtrace (Trace::REFLECTION);

      radiance += refl * sub_trace.render (mirror_ray);
    }

  // Calculate the light from the underlying BRDF (the light supplied to
  // the BRDF is modified by an intervening MirrorCoating BRDF, so it will
  // never use light reflected by specular reflection).
  //
  radiance += Material::render (isec);

  return radiance;
}


// MirrorCoating BRDF

// Remove from SAMPLES any light reflected by perfect specular reflection.
//
void
MirrorCoating::remove_specular_reflection (const Intersect &isec,
					   IllumSampleVec::iterator beg_sample,
					   IllumSampleVec::iterator end_sample)
  const
{
  float medium_ior = isec.trace.medium ? isec.trace.medium->ior : 1;
  const Fresnel fres (medium_ior, ior);

  for (IllumSampleVec::iterator s = beg_sample; s != end_sample; ++s)
    {
      float fres_refl = fres.reflectance (dot (isec.n, s->dir));
      const Color refl = fres_refl * reflectance;
      s->refl *= (1 - refl);
    }
}

// Generate around NUM samples of this BRDF and add them to SAMPLES.
// Return the actual number of samples (NUM is only a suggestion).
//
unsigned
MirrorCoating::gen_samples (const Intersect &isec, unsigned num,
			    IllumSampleVec &samples)
  const
{
  if (! underlying_brdf)
    return 0;

  unsigned base_off = samples.size ();

  // First get the underlying BRDF end_sample generate its native samples.
  //
  num = underlying_brdf->gen_samples (isec, num, samples);

  // Now adjust the samples end_sample remove any light reflected by perfect
  // specular reflection.
  //
  remove_specular_reflection (isec, samples.begin() + base_off, samples.end());

  return num;
}

// Add reflectance information for this BRDF to samples from BEG_SAMPLE
// to END_SAMPLE.
//
void
MirrorCoating::filter_samples (const Intersect &isec, 
			       const IllumSampleVec::iterator &beg_sample,
			       const IllumSampleVec::iterator &end_sample)
  const
{
  if (underlying_brdf)
    {
      remove_specular_reflection (isec, beg_sample, end_sample);

      // Now that we've removed specularly reflected light, apply the
      // underlying BRDF.
      //
      underlying_brdf->filter_samples (isec, beg_sample, end_sample);
    }
  else
    for (IllumSampleVec::iterator s = beg_sample; s != end_sample; s++)
      s->invalid = true;
}

// arch-tag: b895139d-fe9f-414a-9665-3b5e4b8f691a
