// rect-light.h -- Rectangular light
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __RECT_LIGHT_H__
#define __RECT_LIGHT_H__

#include "light.h"
#include "color.h"
#include "pos.h"

namespace Snogray {

class RectLight : public Light
{
public:

  static const unsigned JITTER_STEPS = 5;

  RectLight (const Pos &_pos, const Vec &_side1, const Vec &_side2,
	     const Color &emittance)
    : pos (_pos), side1 (_side1), side2 (_side2),
      power (emittance * cross (_side1, _side2).length ()),
      normal (cross (_side1, _side2).unit ())
  { }

  // Generate (up to) NUM samples of this light and add them to SAMPLES.
  // For best results, they should be distributed according to the light's
  // intensity.
  //
  virtual void gen_samples (const Intersect &isec, SampleRayVec &samples)
    const;

  // Modify the value of the BRDF samples in SAMPLES from FROM to TO,
  // according to the light's intensity in the sample's direction.
  //
  virtual void filter_samples (const Intersect &isec, SampleRayVec &samples,
			       SampleRayVec::iterator from,
			       SampleRayVec::iterator to)
    const;

  // Location and size of the light.
  //
  Pos pos;
  Vec side1, side2;

  // Total emitted power for the entire light.
  //
  Color power;

private:

  Vec normal;
};

}

#endif /* __RECT_LIGHT_H__ */

// arch-tag: d78e86c5-6efa-42ba-811f-332ea423090c
