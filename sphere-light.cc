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

#include "rand.h"
#include "snogmath.h"
#include "intersect.h"

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
      // Note -- the abs here is just to avoid negative numbers caused by
      // floating-point imprecision.
      float z
	= (sqrt (abs (radius * radius - x * x - y * y))
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


// arch-tag: 1caf0ba2-7ec6-4814-be51-b57bbda71fe8
