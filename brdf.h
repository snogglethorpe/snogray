// brdf.h -- Bi-directional reflectance distribution functions
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __BRDF_H__
#define __BRDF_H__

#include <vector>

#include "pos.h"
#include "vec.h"
#include "color.h"
#include "sample-ray.h"
#include "trace-state.h"

namespace Snogray {

class Intersect;

class Brdf
{
public:

  virtual ~Brdf (); // stop gcc bitching

  // Generate samples of this BRDF and add them to SAMPLES.
  //
  virtual void gen_samples (const Intersect &isec, const Color &color,
			    TraceState &tstate, SampleRayVec &samples)
    const = 0;

  // Modify the value of each of the light-samples in SAMPLES according to
  // the BRDF's reflectivity in the sample's direction.
  //
  virtual void filter_samples (const Intersect &isec, const Color &color,
			       TraceState &tstate, SampleRayVec &samples,
			       SampleRayVec::iterator from,
			       SampleRayVec::iterator to)
    const = 0;
};

}

#endif /* __BRDF_H__ */

// arch-tag: 8360ddd7-dc17-40b8-8319-8f6d61fe62bf
