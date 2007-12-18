// sphere-light.h -- Spherical light
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SPHERE_LIGHT_H__
#define __SPHERE_LIGHT_H__

#include "light.h"
#include "color.h"
#include "pos.h"

namespace snogray {

class SphereLight : public Light
{
public:

  SphereLight (const Pos &_pos, float _radius, const Color &_intensity)
    : pos (_pos), radius (_radius), intensity (_intensity) 
  { }

  // Generate around NUM samples of this light and add them to SAMPLES.
  // Return the actual number of samples (NUM is only a suggestion).
  //
  virtual unsigned gen_samples (const Intersect &isec, unsigned num,
				IllumSampleVec &samples)
    const;

  // For every sample from BEG_SAMPLE to END_SAMPLE which intersects this
  // light, and where light is closer than the sample's previously recorded
  // light distance (or the previous distance is zero), overwrite the
  // sample's light-related fields with information from this light.
  //
  virtual void filter_samples (const Intersect &isec, 
			       const IllumSampleVec::iterator &beg_sample,
			       const IllumSampleVec::iterator &end_sample)
    const;

  // Location and size of the light.
  //
  Pos pos;
  dist_t radius;

  // Radiant emittance of this light (W / m^2).
  //
  Color intensity;

private:

  // Return the solid angle subtended by this light, where LIGHT_CENTER_VEC
  // is a vector from the viewer to the light's center.
  //
  float solid_angle (const Vec &light_center_vec) const;
};

}

#endif /* __SPHERE_LIGHT_H__ */

// arch-tag: e40bcb89-44fb-478a-b8b6-c5265c4537d2
