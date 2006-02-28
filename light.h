// light.h -- Light object
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "sample-ray.h"

namespace Snogray {

class Intersect;
class Color;
class Brdf;
class Trace;
class Ray;

class Light
{
public:

  Light () : num (0) { }
  virtual ~Light (); // stop gcc bitching

  // Generate some samples of this light and add them to SAMPLES.
  //
  virtual void gen_samples (const Intersect &isec, SampleRayVec &samples)
    const = 0;

  // Modify the value of the BRDF samples in SAMPLES from FROM to TO,
  // according to the light's intensity in the sample's direction.
  //
  virtual void filter_samples (const Intersect &isec, SampleRayVec &samples,
			       SampleRayVec::iterator from,
			       SampleRayVec::iterator to)
    const = 0;

  // Adjust this light's intensity by a factor of SCALE.
  //
  virtual void scale_intensity (float scale) = 0;

  // Each light has a number, which we use as a index to access various
  // data structures referring to lights.
  //
  unsigned num;
};

}

#endif /* __LIGHT_H__ */

// arch-tag: 07d0a36e-d44f-44f8-bb69-e57c9681de14
