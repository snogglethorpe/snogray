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
#include "scene/scene.h"
#include "intersect/intersect.h"
#include "geometry/cone-sample.h"
#include "geometry/tangent-disk-sample.h"

#include "far-light.h"

using namespace snogray;


// FarLight::Sampler

class FarLight::Sampler : public Light::Sampler
{
public:

  Sampler (const FarLight &_light, const BBox &scene_bbox)
    : light (_light),
      scene_center (scene_bbox.center ()),
      scene_radius (scene_bbox.radius ())
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
  virtual bool is_point_light () const { return light.cos_half_angle == 1; }

  // Evaluate this environmental light in direction DIR (in world-coordinates).
  //
  virtual Color eval_environ (const Vec &dir) const;

private:

  const FarLight &light;

  // Center and radius of a bounding sphere for the entire scene.
  //
  Pos scene_center;
  dist_t scene_radius;
};


// Add light-samplers for this light in SCENE to SAMPLERS.  Any
// samplers added become owned by the owner of SAMPLERS, and will be
// destroyed when it is.
//
void
FarLight::add_light_samplers (const Scene &scene,
			      std::vector<const Light::Sampler *> &samplers)
  const
{
  samplers.push_back (new Sampler (*this, scene.bbox ()));
}



// FarLight::sample

// Return a sample of this light from the viewpoint of ISEC (using a
// surface-normal coordinate system, where the surface normal is
// (0,0,1)), based on the parameter PARAM.
//
Light::Sampler::Sample
FarLight::Sampler::sample (const Intersect &isec, const UV &param) const
{
  // Sample a cone pointing at our light.
  //
  Vec s_dir
    = isec.normal_frame.to (light.frame.from (cone_sample (light.cos_half_angle,
							   param)));
  float pdf
    = light.cos_half_angle == 1 ? 1 : cone_sample_pdf (light.cos_half_angle);

  if (isec.cos_n (s_dir) > 0 && isec.cos_geom_n (s_dir) > 0)
    return Sample (light.intensity, pdf, s_dir, 0);

  return Sample ();
}

// Return a "free sample" of this light.
//
Light::Sampler::FreeSample
FarLight::Sampler::sample (const UV &param, const UV &dir_param) const
{
  // Note that the sample position and direction are decoupled, as a
  // far-light is "really really far away" from the scene.  A given
  // sample point will appear in the same direction from any location
  // in the scene.

  Vec s_dir = light.frame.from (cone_sample (light.cos_half_angle, dir_param));
  Pos s_pos = tangent_disk_sample (scene_center, scene_radius, s_dir, param);

  // Adjust pdf to include disk sampling.
  //
  float pdf
    = light.cos_half_angle == 1 ? 1 : cone_sample_pdf (light.cos_half_angle);
  float s_pdf = pdf / (float (scene_radius * scene_radius) * PIf);

  return FreeSample (light.intensity, s_pdf, s_pos, -s_dir);
}



// FarLight::Sampler::eval

// Evaluate this light in direction DIR from the viewpoint of ISEC
// (using a surface-normal coordinate system, where the surface normal
// is (0,0,1)).
//
Light::Sampler::Value
FarLight::Sampler::eval (const Intersect &isec, const Vec &dir) const
{
  if (light.cos_half_angle < 1)
    {
      Vec light_normal_dir = isec.normal_frame.to (light.frame.z);
      float pdf = cone_sample_pdf (light.cos_half_angle);

      if (cos_angle (dir, light_normal_dir) >= light.cos_half_angle)
	return Value (light.intensity, pdf, 0);
    }

  return Value ();
}

// Evaluate this environmental light in direction DIR (in world-coordinates).
//
Color
FarLight::Sampler::eval_environ (const Vec &dir) const
{
  if (light.cos_half_angle < 1)
    {
      // Cosine of the angle between DIR and the direction of this light.
      //
      float cos_light_dir = cos_angle (dir, light.frame.z);

      // If COS_LIGHT_DIR is greater than COS_HALF_ANGLE, then DIR must be
      // within ANGLE/2 of the light direction, so return the light's
      // color; otherwise just return 0.
      //
      if (cos_light_dir > light.cos_half_angle)
	return light.intensity;
    }

  return 0;
}


// arch-tag: 879b496d-2a8d-4a7e-8d0a-f92d67d4f165
