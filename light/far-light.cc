// far-light.cc -- Light at infinite distance
//
//  Copyright (C) 2005-2008, 2010, 2012, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "geometry/bbox.h"
#include "scene.h"
#include "intersect.h"
#include "geometry/cone-sample.h"
#include "geometry/tangent-disk-sample.h"

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
  // Sample a cone pointing at our light.
  //
  Vec s_dir
    = isec.normal_frame.to (frame.from (cone_sample (cos_half_angle, param)));
  float pdf = cos_half_angle == 1 ? 1 : cone_sample_pdf (cos_half_angle);

  if (isec.cos_n (s_dir) > 0 && isec.cos_geom_n (s_dir) > 0)
    return Sample (intensity, pdf, s_dir, 0);

  return Sample ();
}



// Return a "free sample" of this light.
//
Light::FreeSample
FarLight::sample (const UV &param, const UV &dir_param) const
{
  // Note that the sample position and direction are decoupled, as a
  // far-light is "really really far away" from the scene.  A given
  // sample point will appear in the same direction from any location
  // in the scene.

  Vec s_dir = frame.from (cone_sample (cos_half_angle, dir_param));
  Pos s_pos = tangent_disk_sample (scene_center, scene_radius, s_dir, param);

  // Adjust pdf to include disk sampling.
  //
  float pdf = cos_half_angle == 1 ? 1 : cone_sample_pdf (cos_half_angle);
  float s_pdf = pdf / (float (scene_radius * scene_radius) * PIf);

  return FreeSample (intensity, s_pdf, s_pos, -s_dir);
}


// FarLight::eval

// Evaluate this light in direction DIR from the viewpoint of ISEC
// (using a surface-normal coordinate system, where the surface normal
// is (0,0,1)).
//
Light::Value
FarLight::eval (const Intersect &isec, const Vec &dir) const
{
  if (cos_half_angle < 1)
    {
      Vec light_normal_dir = isec.normal_frame.to (frame.z);
      float pdf = cone_sample_pdf (cos_half_angle);

      if (cos_angle (dir, light_normal_dir) >= cos_half_angle)
	return Value (intensity, pdf, 0);
    }

  return Value ();
}



// Evaluate this environmental light in direction DIR (in world-coordinates).
//
Color
FarLight::eval_environ (const Vec &dir) const
{
  if (cos_half_angle < 1)
    {
      // Cosine of the angle between DIR and the direction of this light.
      //
      float cos_light_dir = cos_angle (dir, frame.z);

      // If COS_LIGHT_DIR is greater than COS_HALF_ANGLE, then DIR must be
      // within ANGLE/2 of the light direction, so return the light's
      // color; otherwise just return 0.
      //
      if (cos_light_dir > cos_half_angle)
	return intensity;
    }

  return 0;
}



// Do any scene-related setup for this light.  This is is called once
// after the entire scene has been loaded.
//
void
FarLight::scene_setup (const Scene &scene)
{
  // Record the center and radius of a bounding sphere for the scene.

  BBox scene_bbox = scene.surfaces.bbox ();

  scene_center = scene_bbox.min + scene_bbox.extent () / 2;
  scene_radius = scene_bbox.extent ().length () / 2;
}


// arch-tag: 879b496d-2a8d-4a7e-8d0a-f92d67d4f165
