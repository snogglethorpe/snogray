// far-light.h -- Light at infinite distance
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __FAR_LIGHT_H__
#define __FAR_LIGHT_H__

#include "snogmath.h"
#include "light.h"
#include "color.h"
#include "pos.h"

namespace Snogray {

class FarLight : public Light
{
public:

  static const unsigned JITTER_STEPS = 5;

  FarLight (const Vec &_dir, dist_t _radius, const Color &col)
    : dir (_dir), radius (_radius), color (col), max_cos (cos (atan (_radius))),
      steps_radius (double (JITTER_STEPS) / 2)
  { init (); }

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

  Vec dir;
  dist_t radius;

  Color color;

private:

  void init ();
  
  Vec u, v;
  Vec u_inc, v_inc;

  dist_t max_cos;

  double steps_radius;

  Color::component_t num_lights_scale;
};

}

#endif /* __FAR_LIGHT_H__ */

// arch-tag: 0691dd09-998d-4cdf-b5e9-da71aed2ec41
