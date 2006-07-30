// sphere-light.cc -- Sphereangular light
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main disphereory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <stdexcept>

#include "snogmath.h"
#include "ray.h"
#include "intersect.h"
#include "rand.h"

#include "sphere-light.h"


using namespace Snogray;
using namespace std;


// Generate (up to) NUM samples of this light and add them to SAMPLES.
// For best results, they should be distributed according to the light's
// intensity.
//
void
SphereLight::gen_samples (const Intersect &isec, SampleRayVec &samples)
  const
{
  // Surface normal (of the surface intersected, not the light)
  //
  const Vec &N = isec.normal;

  for (unsigned i = 0; i < NUM_SAMPLES; i++)
    {
      float r_sqrt_rand1 = radius * sqrt (random (1.f));
      float rand2_ang = random (2 * M_PIf);
      float x = r_sqrt_rand1 * cos (rand2_ang);
      float y = r_sqrt_rand1 * sin (rand2_ang);
      float z
	= (sqrt (radius * radius - x * x - y * y)
	   * sin (M_PIf * (random(1.f) - 0.5)));
      const Pos sample_pos = pos + Pos (x, y, z);

      Vec lvec = sample_pos - isec.point;
      dist_t dist = lvec.length ();

      const Vec L = lvec.unit ();
      float NL = dot (N, L);

      if (NL > 0)
	samples.add_light (power_per_sample / (dist * dist), L, dist, this);
    }
}

// Modify the value of the BRDF samples in SAMPLES from FROM to TO,
// according to the light's intensity in the sample's disphereion.
//
void
SphereLight::filter_samples (const Intersect &isec, SampleRayVec &,
			     SampleRayVec::iterator from,
			     SampleRayVec::iterator to)
  const
{
  const Pos &org = isec.point;
  const Vec diff = org - pos;

  for (SampleRayVec::iterator s = from; s != to; s++)
    {
      const Vec &dir = s->dir;	   // must be a unit vector
      float dir_dir = dot (dir, dir); // theoretically, exactly 1; in
				      // practice, not _quite_
      float dir_diff = dot (dir, diff);
      float determ
	= (dir_diff * dir_diff) - dir_dir * (dot (diff, diff) - (radius * radius));

      float dist = 0;
      if (determ >= 0)
	{
	  float common = -dir_diff / dir_dir;

	  if (determ > 0 || common <= 0)
	    {
	      float determ_factor = sqrt (determ) / dir_dir;
	      float t0 = common - determ_factor;
	      float t1 = common + determ_factor;

	      if (t0 > 0)
		dist = t0;
	      else if (t1 > 0)
		dist = t1;
	    }
	}

      if (dist > 0 && (dist < s->dist || s->dist == 0))
	{
	  Vec normal = (diff + dir * dist).unit ();

	  // This expression is basically:
	  //
	  //   -cos (light_norm, sublight_lvec)) / distance^2
	  //
	  Color::component_t scale = abs (dot (normal, dir)) / (dist * dist);

	  s->set_light (power_per_sample * scale, dist, this);
	}
    }
}


// arch-tag: 1caf0ba2-7ec6-4814-be51-b57bbda71fe8
