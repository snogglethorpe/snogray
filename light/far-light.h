// far-light.h -- Light at infinite distance
//
//  Copyright (C) 2005-2008, 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_FAR_LIGHT_H
#define SNOGRAY_FAR_LIGHT_H

#include "util/snogmath.h"
#include "color/color.h"
#include "geometry/pos.h"

#include "light.h"


namespace snogray {


// A light at an "infinite" distance.
//
class FarLight : public Light
{
public:

  // ANGLE is the apparent (linear) angle subtended by of the light.
  // INTENSITY is the amount of light emitted per steradian.
  //
  // As a special case, when ANGLE is exactly 0 -- creating an
  // infinitely-far-away point-light -- then INTENSITY is the
  // absolute intensity, not the intensity per steradian.
  //
  FarLight (const Vec &_dir, float angle, const Color &_intensity)
    : intensity (_intensity), frame (_dir.unit ()),
      // To ensure that if ANGLE is zero, COS_HALF_ANGLE becomes
      // exactly 1 (because we explicitly compare with that), don't
      // trust the cos function to return exactly the right result.
      cos_half_angle (angle == 0 ? 1 : cos (angle / 2))
  { }

  // Add light-samplers for this light in SCENE to SAMPLERS.  Any
  // samplers added become owned by the owner of SAMPLERS, and will be
  // destroyed when it is.
  //
  virtual void add_light_samplers (
		 const Scene &scene,
		 std::vector<const Light::Sampler *> &samplers)
    const;

  // Transform the geometry of this light by XFORM.
  //
  virtual void transform (const Xform &xform);


private:

  class Sampler;

  Color intensity;
  
  // Frame of reference pointing at this light from the origin.
  //
  Frame frame;

  // The cosine of half the angle subtended by this light's cone.
  //
  float cos_half_angle;
};


}

#endif // SNOGRAY_FAR_LIGHT_H

// arch-tag: 0691dd09-998d-4cdf-b5e9-da71aed2ec41
