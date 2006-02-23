// far-light.cc -- Light at infinite distance
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "ray.h"
#include "intersect.h"
#include "rand.h"
#include "scene.h"

#include "far-light.h"

using namespace Snogray;

void
FarLight::init ()
{
  Vec up (0, 1, 0);

  Vec u0 = cross (dir, up);

  // If DIR was the same as UP, U0 ends up zero-length, so retry with
  // another UP.
  //
  if (u0.length () < Eps)
    u0 = cross (dir, Vec (1, 0, 0));

  u = cross (dir, u0).unit () * radius;
  v = cross (dir, u).unit ()  * radius;

  u_inc = u * 2 / JITTER_STEPS;
  v_inc = v * 2 / JITTER_STEPS;

  dist_t r_sq = steps_radius * steps_radius;
  double v_offs = -steps_radius;

  // Pre-scale the brightness so that each sublight contributes 1 / NUM_LIGHTS
  // to the total brightness.  We have to count the number of lights
  // that actually contribute.

  double num_lights = 0;

  for (unsigned i = 0; i < JITTER_STEPS; i++)
    {
      double u_offs = -steps_radius;

      for (unsigned j = 0; j < JITTER_STEPS; j++)
	{
	  if ((u_offs + 0.5) * (u_offs + 0.5) + (v_offs + 0.5) * (v_offs + 0.5)
	      <= r_sq)
	    num_lights++;

	  u_offs += 1;
	}

      v_offs += 1;
    }

  num_lights_scale = 1.0 / num_lights;
}

// Generate (up to) NUM samples of this light and add them to SAMPLES.
// For best results, they should be distributed according to the light's
// intensity.
//
void
FarLight::gen_samples (const Intersect &isec, TraceState &tstate,
		       SampleRayVec &samples)
  const
{
  // Surface normal
  //
  const Vec &N = isec.normal;

  // First detect cases where the light isn't visible at all, by
  // examining the dot product of the surface normal with rays to the
  // four corners of the light.
  //
  if (dot (N, dir + u) > 0 || dot (N, dir - u) > 0
      || dot (N, dir + v) > 0 || dot (N, dir - v) > 0)
    {
      Color samp_color = color * num_lights_scale;
      dist_t r_sq = steps_radius * steps_radius;
      double v_offs = -steps_radius;

      for (unsigned i = 0; i < JITTER_STEPS; i++)
	{
	  double u_offs = -steps_radius;

	  for (unsigned j = 0; j < JITTER_STEPS; j++)
	    if ((u_offs + 0.5) * (u_offs + 0.5)
		+ (v_offs + 0.5) * (v_offs + 0.5)
		<= r_sq)
	      {
		const Vec jitter
		  = u_inc * (u_offs + random (0, 1))
		  + v_inc * (v_offs + random (0, 1));

		const Vec L = (dir + jitter).unit();
		float NL = dot (N, L);

		if (NL > 0)
		  samples.add_light (samp_color, L, Scene::DEFAULT_HORIZON,
				     this);

		u_offs += 1;
	      }

	  v_offs += 1;
	}
    }
}

// Modify the value of the BRDF samples in SAMPLES from FROM to TO,
// according to the light's intensity in the sample's difarion.
//
void
FarLight::filter_samples (const Intersect &isec, TraceState &tstate,
			  SampleRayVec &samples,
			  SampleRayVec::iterator from,
			  SampleRayVec::iterator to)
  const
{
  for (SampleRayVec::iterator s = from; s != to; s++)
    if (s->dist == 0 && dot (dir, s->dir) <= max_cos)
      s->set_light (color, Scene::DEFAULT_HORIZON, this);
}

// Adjust this light's intensity by a factor of SCALE.
//
void
FarLight::scale_intensity (float scale)
{
  color *= scale;
}

// arch-tag: 879b496d-2a8d-4a7e-8d0a-f92d67d4f165
