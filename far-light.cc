// far-light.cc -- Light at infinite distance
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "intersect.h"

#include "far-light.h"

using namespace snogray;


// FarLight::sample

// Return a sample of this light from the viewpoint of ISEC (using a
// surface-normal coordinate system, where the surface normal is
// (0,0,1)), based on the parameter PARAM.
//
Light::Sample
FarLight::sample (const Intersect &isec, const UV &param) const
{
  // First detect cases where the light isn't visible at all given the
  // ISEC's surface normal.
  //
  float cos_n_dir_angle = isec.cos_n (isec.normal_frame.to (frame.z));
  float n_dir_angle = acos (clamp (cos_n_dir_angle, -1.f, 1.f));
  float min_angle = n_dir_angle - angle / 2;

  if (min_angle < 2 * PIf)
    {
      // Sample onto the upper part of a cylinder.  The total height of
      // the cylinder is 2 (from -1 to 1), and the height of the sample
      // area correponds to ANGLE (so that if ANGLE is 4*PI, the entire
      // cylinder surface, from z -1 to 1, will be sampled).
      //
      float z = 1 - param.u * angle * 0.5f * INV_PIf;
      float r = sqrt (1 - z * z);
      float phi = param.v * 2 * PIf;
      float x = r * cos (phi), y = r * sin (phi);
      Vec s_dir = isec.normal_frame.to (frame.from (Vec (x, y, z)));

      if (isec.cos_n (s_dir) > 0 && isec.cos_geom_n (s_dir) > 0)
	return Sample (intensity, pdf, s_dir, 0);
    }

  return Sample ();
}


// FarLight::eval

// Evaluate this light in direction DIR from the viewpoint of ISEC (using
// a surface-normal coordinate system, where the surface normal is
// (0,0,1)).
//
Light::Value
FarLight::eval (const Intersect &isec, const Vec &dir) const
{
  Vec light_normal_dir = isec.normal_frame.to (frame.z);

  if (dot (dir, light_normal_dir) >= min_cos)
    return Value (intensity, pdf, 0);
  else
    return Value ();
}


// arch-tag: 879b496d-2a8d-4a7e-8d0a-f92d67d4f165
