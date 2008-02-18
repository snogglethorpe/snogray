// sphere-light.cc -- Sphereangular light
//
//  Copyright (C) 2006, 2007, 2008  Miles Bader <miles@gnu.org>
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
#include "grid-iter.h"
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



// Generate around NUM samples of this light and add them to SAMPLES.
// Return the actual number of samples (NUM is only a suggestion).
//
unsigned
SphereLight::gen_samples (const Intersect &isec, unsigned num,
			  IllumSampleVec &samples)
  const
{
  // Offset of the center of the light sphere from the intersection origin,
  // in the intersection's normal frame of reference.
  //
  const Vec light_center_vec = isec.normal_frame.to (pos);

  // If this light is "below the horizon", and so can have no effect,
  // return immediately.
  //
  if (light_center_vec.z < -radius)
    return 0;

  // The distribution used here is constant over a solid angle when viewed
  // by an external observer, meaning that it also has a constant pdf equal
  // to 1 / solid_angle.
  //
  float pdf = 1 / solid_angle (light_center_vec);

  GridIter grid_iter (num);

  float u, v;
  while (grid_iter.next (u, v))
    {
      // The following distribution which is constant over a solid angle
      // when viewed by an external observer.  The algorithm is from the
      // paper "Lightcuts: a scalable approach to illumination", by Bruce
      // Walters, et al.
      //
      float r_sqrt_rand1 = radius * sqrt (u);
      float rand2_ang = v * 2 * PIf;
      float x = r_sqrt_rand1 * cos (rand2_ang);
      float y = r_sqrt_rand1 * sin (rand2_ang);
      // Note -- the abs here is just to avoid negative numbers caused by
      // floating-point imprecision.
      float z
	= (sqrt (abs (radius * radius - x * x - y * y))
	   * sin (PIf * (random(1.f) - 0.5)));

      // A vector from the intersection origin to the point (X, Y, Z)
      // within the sphere, in the intersection's normal frame of
      // reference.
      //
      const Vec s_vec = light_center_vec + Vec (x, y, z);

      if (isec.cos_n (s_vec) > 0)
	{
	  const Vec s_dir = s_vec.unit ();

	  // The "real" distance must terminate at the surface of the
	  // sphere, so we need to do that calculation too...
	  //
	  dist_t dist = sphere_intersect (radius, -light_center_vec, s_dir);

	  samples.push_back (IllumSample (s_dir, intensity, pdf, dist, this));
	}
    }

  return grid_iter.num_samples ();
}



// For every sample from BEG_SAMPLE to END_SAMPLE which intersects this
// light, and where light is closer than the sample's previously recorded
// light distance (or the previous distance is zero), overwrite the
// sample's light-related fields with information from this light.
//
void
SphereLight::filter_samples (const Intersect &isec, 
			     const IllumSampleVec::iterator &beg_sample,
			     const IllumSampleVec::iterator &end_sample)
  const
{
  // Offset of the center of the light sphere from the intersection origin,
  // in the intersection's normal frame of reference.
  //
  const Vec light_center_vec = isec.normal_frame.to (pos);

  // If this light is "below the horizon", and so can have no effect,
  // return immediately.
  //
  if (light_center_vec.z < -radius)
    return;

  // The distribution used here is constant over a solid angle when viewed
  // by an external observer, meaning that it also has a constant pdf equal
  // to 1 / solid_angle.
  //
  float pdf = 1 / solid_angle (light_center_vec);

  for (IllumSampleVec::iterator s = beg_sample; s != end_sample; s++)
    {
      float dist = sphere_intersect (radius, -light_center_vec, s->dir);

      if (dist > 0 && (dist < s->light_dist || s->light_dist == 0))
	{
	  s->light_pdf = pdf;
	  s->light_val = intensity; //XXX * pdf;
	  s->light_dist = dist;
	  s->light = this;
	}
    }
}


// arch-tag: 1caf0ba2-7ec6-4814-be51-b57bbda71fe8
