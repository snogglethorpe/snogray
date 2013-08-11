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

#include "snogmath.h"
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
      cos_half_angle (angle == 0 ? 1 : cos (angle / 2)),
      scene_radius (0)
  { }

  // Return a sample of this light from the viewpoint of ISEC (using a
  // surface-normal coordinate system, where the surface normal is
  // (0,0,1)), based on the parameter PARAM.
  //
  virtual Sample sample (const Intersect &isec, const UV &param) const;

  // Return a "free sample" of this light.
  //
  virtual FreeSample sample (const UV &param, const UV &dir_param) const;

  // Evaluate this light in direction DIR from the viewpoint of ISEC (using
  // a surface-normal coordinate system, where the surface normal is
  // (0,0,1)).
  //
  virtual Value eval (const Intersect &isec, const Vec &dir) const;

  // Return true if this is an "environmental" light, not associated
  // with any surface.
  //
  virtual bool is_environ_light () const { return true; }

  // Return true if this is a point light.
  //
  virtual bool is_point_light () const { return cos_half_angle == 1; }

  // Evaluate this environmental light in direction DIR (in world-coordinates).
  //
  virtual Color eval_environ (const Vec &dir) const;

  // Do any scene-related setup for this light.  This is is called once
  // after the entire scene has been loaded.
  //
  virtual void scene_setup (const Scene &scene);

  Color intensity;

private:
  
  // Frame of reference pointing at this light from the origin.
  //
  Frame frame;

  // The cosine of half the angle subtended by this light's cone.
  //
  float cos_half_angle;

  // Center and radius of a bounding sphere for the entire scene.
  //
  Pos scene_center;
  dist_t scene_radius;
};


}

#endif // SNOGRAY_FAR_LIGHT_H

// arch-tag: 0691dd09-998d-4cdf-b5e9-da71aed2ec41
