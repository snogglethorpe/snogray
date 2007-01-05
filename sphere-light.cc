// sphere-light.cc -- Sphereangular light
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main disphereory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "snogmath.h"
#include "intersect.h"
#include "grid-iter.h"
#include "sphere-isec.h"

#include "sphere-light.h"


using namespace snogray;


// Return the solid angle subtended by this sphere from the point of view
// of an external observer at OBSERVER.
//
float
SphereLight::solid_angle (const Pos &observer) const
{
  float dist = (pos - observer).length ();
  return 2 * M_PIf * (1 - cos (asin (radius / dist)));
}



// Generate around NUM samples of this light and add them to SAMPLES.
// Return the actual number of samples (NUM is only a suggestion).
//
unsigned
SphereLight::gen_samples (const Intersect &isec, unsigned num,
			  IllumSampleVec &samples)
  const
{
  // Since the sphere intersection calculation uses a sphere centered at
  // the origin, we offset the sample start point to compensate.
  //
  const Vec &s_isec_offs = isec.pos - pos;

  // The distribution used here is constant over a solid angle when viewed
  // by an external observer, meaning that it also has a constant pdf equal
  // to 1 / solid_angle.
  //
  float pdf = 1 / solid_angle (isec.pos);

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
      float rand2_ang = v * 2 * M_PIf;
      float x = r_sqrt_rand1 * cos (rand2_ang);
      float y = r_sqrt_rand1 * sin (rand2_ang);
      // Note -- the abs here is just to avoid negative numbers caused by
      // floating-point imprecision.
      float z
	= (sqrt (abs (radius * radius - x * x - y * y))
	   * sin (M_PIf * (random(1.f) - 0.5)));

      // End-point of sample, which is _inside_ the sphere light (not on
      // the surface).
      //
      const Pos s_end = pos + Vec (x, y, z);
      const Vec s_vec = s_end - isec.pos;

      if (isec.cos_n (s_vec) > 0)
	{
	  const Vec s_dir = s_vec.unit ();

	  // The "real" distance must terminate at the surface of the
	  // sphere, so we need to do that calculation too...
	  //
	  dist_t dist = sphere_intersect (radius, s_isec_offs, s_dir);

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
  // Since the sphere intersection calculation uses a sphere centered at
  // the origin, we offset the sample start point to compensate.
  //
  const Vec &s_isec_offs = isec.pos - pos;

  // The distribution used here is constant over a solid angle when viewed
  // by an external observer, meaning that it also has a constant pdf equal
  // to 1 / solid_angle.
  //
  float pdf = 1 / solid_angle (isec.pos);

  for (IllumSampleVec::iterator s = beg_sample; s != end_sample; s++)
    {
      float dist = sphere_intersect (radius, s_isec_offs, s->dir);

      if (dist > 0 && (dist < s->dist || s->dist == 0))
	{
	  s->light_pdf = pdf;
	  s->val = intensity; //XXX * pdf;
	  s->dist = dist;
	  s->light = this;
	}
    }
}


// arch-tag: 1caf0ba2-7ec6-4814-be51-b57bbda71fe8
