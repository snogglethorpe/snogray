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

  Vec u0 = dir.cross (up);

  // If DIR was the same as UP, U0 ends up zero-length, so retry with
  // another UP.
  //
  if (u0.length () < Eps)
    u0 = dir.cross (Vec (1, 0, 0));

  u = dir.cross (u0).unit () * radius;
  v = dir.cross (u).unit ()  * radius;

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

  color /= num_lights;
}



// Return the color of the surface at ISEC, with nominal color
// SURFACE_COLOR, as lit by this light with lighting-model LIGHT_MODEL.
//
Color
FarLight::illum (const Intersect &isec, const Color &surface_color,
		 const LightModel &light_model, TraceState &tstate)
  const
{
  if (isec.normal.dot (dir + u) > 0
      || isec.normal.dot (dir - u) > 0
      || isec.normal.dot (dir + v) > 0
      || isec.normal.dot (dir - v) > 0)
    {
      Color illum;
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

		const Ray light_ray (isec.point, (dir + jitter).unit(),
				     Scene::DEFAULT_HORIZON);

		illum += ray_illum (light_ray, color, isec,
				    surface_color, light_model, tstate);

		u_offs += 1;
	      }

	  v_offs += 1;
	}

      return illum;
    }
  else
    return Color (0, 0, 0);
}

// Adjust this light's intensity by a factor of SCALE.
//
void
FarLight::scale_intensity (float scale)
{
  color *= scale;
}

// arch-tag: 879b496d-2a8d-4a7e-8d0a-f92d67d4f165
