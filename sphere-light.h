// sphere-light.h -- Spherical light
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main disphereory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SPHERE_LIGHT_H__
#define __SPHERE_LIGHT_H__

#include "light.h"
#include "color.h"
#include "pos.h"

namespace Snogray {

class SphereLight : public Light
{
public:

  static const unsigned NUM_SAMPLES = 25;

  SphereLight (const Pos &_pos, float _radius, const Color &emittance)
    : pos (_pos), radius (_radius),
      power_per_sample (emittance * (4.f * float (M_PI) * _radius * _radius)
			/ NUM_SAMPLES)
  { }

  // Generate (up to) NUM samples of this light and add them to SAMPLES.
  // For best results, they should be distributed according to the light's
  // intensity.
  //
  virtual void gen_samples (const Intersect &isec, SampleRayVec &samples)
    const;

  // Modify the value of the BRDF samples in SAMPLES from FROM to TO,
  // according to the light's intensity in the sample's disphereion.
  //
  virtual void filter_samples (const Intersect &isec, SampleRayVec &samples,
			       SampleRayVec::iterator from,
			       SampleRayVec::iterator to)
    const;

  // Location and size of the light.
  //
  Pos pos;
  float radius;

  // Power emitted per light sample.
  //
  Color power_per_sample;
};

}

#endif /* __SPHERE_LIGHT_H__ */

// arch-tag: e40bcb89-44fb-478a-b8b6-c5265c4537d2
