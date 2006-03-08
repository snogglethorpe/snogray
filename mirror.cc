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

#include <cmath>

#include "mirror.h"

#include "intersect.h"
#include "scene.h"

using namespace Snogray;

Color
Mirror::render (const Intersect &isec) const
{
  float cos_refl_angle = dot (isec.normal, isec.viewer);
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
      Vec mirror_dir = isec.ray.dir.reflection (isec.normal);
      Ray mirror_ray (isec.point, mirror_dir);
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
					   SampleRayVec &samples,
					   SampleRayVec::iterator from,
					   SampleRayVec::iterator to)
  const
{
  float medium_ior = isec.trace.medium ? isec.trace.medium->ior : 1;
  const Fresnel fres (medium_ior, ior);

  for (SampleRayVec::iterator s = from; s != to; s++)
    {
      float fres_refl = fres.reflectance (dot (isec.normal, s->dir));
      const Color refl = fres_refl * reflectance;
      s->set_refl (1 - refl);
    }
}

// Generate (up to) NUM samples of this BRDF and add them to SAMPLES.
// For best results, they should be distributed according to the BRDF's
// importance function.
//
void
MirrorCoating::gen_samples (const Intersect &isec, SampleRayVec &samples)
  const
{
  if (underlying_brdf)
    {
      // First get the underlying BRDF to generate its native samples.
      //
      underlying_brdf->gen_samples (isec, samples);

      // Now adjust the samples to remove any light reflected by perfect
      // specular reflection.
      //
      remove_specular_reflection
	(isec, samples, samples.begin(), samples.end());
    }
}

// Modify the value of each of the light-samples in SAMPLES according to
// the BRDF's reflectivity in the sample's direction.
//
void
MirrorCoating::filter_samples (const Intersect &isec, SampleRayVec &samples,
			      SampleRayVec::iterator from,
			      SampleRayVec::iterator to)
  const
{
  if (underlying_brdf)
    {
      remove_specular_reflection (isec, samples, from, to);

      // Now that we've removed specularly reflected light, apply the
      // underlying BRDF.
      //
      underlying_brdf->filter_samples (isec, samples, from, to);
    }
  else
    for (SampleRayVec::iterator s = from; s != to; s++)
      s->invalidate ();
}

// arch-tag: b895139d-fe9f-414a-9665-3b5e4b8f691a
