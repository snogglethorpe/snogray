// point-light.h -- Point light
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __POINT_LIGHT_H__
#define __POINT_LIGHT_H__

#include "light.h"
#include "color.h"
#include "pos.h"

namespace Snogray {

class PointLight : public Light
{
public:

  PointLight (const Pos &_pos, const Color &col) : pos (_pos), color (col) { }

  // Generate (up to) NUM samples of this light and add them to SAMPLES.
  // For best results, they should be distributed according to the light's
  // intensity.
  //
  virtual void gen_samples (const Intersect &isec, TraceState &tstate,
			    SampleRayVec &samples)
    const;

  // Modify the value of the BRDF samples in SAMPLES from FROM to TO,
  // according to the light's intensity in the sample's direction.
  //
  virtual void filter_samples (const Intersect &isec, TraceState &tstate,
			       SampleRayVec &samples,
			       SampleRayVec::iterator from,
			       SampleRayVec::iterator to)
    const;

  // Adjust this light's intensity by a factor of SCALE.
  //
  virtual void scale_intensity (float scale);

  Pos pos;
  Color color;
};

}

#endif /* __POINT_LIGHT_H__ */

// arch-tag: de5d75ba-f8bd-4dde-a729-37ea8f37a7cc
