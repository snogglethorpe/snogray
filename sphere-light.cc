// sphere-light.cc -- Spherical light
//
//  Copyright (C) 2006, 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "snogmath.h"
#include "intersect.h"
#include "sphere-isec.h"

#include "sphere-light.h"


using namespace snogray;


// Return the solid angle subtended by this light, where LIGHT_CENTER_VEC
// is a vector from the viewer to the light's center.
//
float
SphereLight::solid_angle (const Vec &light_center_vec) const
{
  float dist = light_center_vec.length ();
  if (dist < radius)
    return 4 * PIf;
  else
    return 2 * PIf * (1 - cos (asin (radius / dist)));
}


// SphereLight::sample

// Return a sample of this light from the viewpoint of ISEC (using a
// surface-normal coordinate system, where the surface normal is
// (0,0,1)), based on the parameter PARAM.
//
Light::Sample
SphereLight::sample (const Intersect &isec, const UV &param) const
{
  // Offset of the center of the light sphere from the intersection origin,
  // in the intersection's normal frame of reference.
  //
  const Vec light_center_vec = isec.normal_frame.to (pos);

  // Only do anything if this light is "above the horizon", and so can
  // conceivably be seen from ISEC.
  //
  if (light_center_vec.z >= -radius)
    {
      // The distribution used here is constant over a solid angle when
      // viewed by an external observer, meaning that it also has a
      // constant pdf equal to 1 / solid_angle.
      //
      float pdf = 1 / solid_angle (light_center_vec);

      // The following distribution which is constant over a solid angle
      // when viewed by an external observer.  The algorithm is from the
      // paper "Lightcuts: a scalable approach to illumination", by Bruce
      // Walters, et al.
      //
      float r_sqrt_rand1 = radius * sqrt (param.u);
      float rand2_ang = param.v * 2 * PIf;
      float x = r_sqrt_rand1 * cos (rand2_ang);
      float y = r_sqrt_rand1 * sin (rand2_ang);
      // Note -- the abs here is just to avoid negative numbers caused by
      // floating-point imprecision.
      float z
	= (sqrt (abs (radius * radius - x * x - y * y))
	   * sin (PIf * (isec.context.random() - 0.5f)));

      // A vector from the intersection origin to the point (X, Y, Z)
      // within the sphere, in the intersection's normal frame of
      // reference.
      //
      const Vec s_vec = light_center_vec + Vec (x, y, z);

      if (isec.cos_n (s_vec) > 0 && isec.cos_geom_n (s_vec) > 0)
	{
	  const Vec s_dir = s_vec.unit ();

	  // The "real" distance must terminate at the surface of the
	  // sphere, so we need to do that calculation too...
	  //
	  dist_t dist = sphere_intersect (radius, -light_center_vec, s_dir);

	  return Sample (intensity, pdf, s_dir, dist);
	}
    }

  return Sample ();
}


// SphereLight::eval

// Evaluate this light in direction DIR from the viewpoint of ISEC (using
// a surface-normal coordinate system, where the surface normal is
// (0,0,1)).
//
Light::Value
SphereLight::eval (const Intersect &isec, const Vec &dir) const
{
  // Offset of the center of the light sphere from the intersection origin,
  // in the intersection's normal frame of reference.
  //
  const Vec light_center_vec = isec.normal_frame.to (pos);

  // Only do anything if this light is "above the horizon", and so can
  // conceivably be seen from ISEC.
  //
  if (light_center_vec.z >= -radius)
    {
      float dist = sphere_intersect (radius, -light_center_vec, dir);

      if (dist)
	{
	  // The distribution used here is constant over a solid angle when
	  // viewed by an external observer, meaning that it also has a
	  // constant pdf equal to 1 / solid_angle.
	  //
	  float pdf = 1 / solid_angle (light_center_vec);

	  return Value (intensity, pdf, dist);
	}
    }

  return Value ();
}


// arch-tag: 1caf0ba2-7ec6-4814-be51-b57bbda71fe8
