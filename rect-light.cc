// rect-light.cc -- Rectangular light
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

#include "rect-light.h"

using namespace Snogray;

// Return the color of the surface at ISEC, with nominal color
// SURFACE_COLOR, as lit by this light with lighting-model LIGHT_MODEL.
//
Color
RectLight::illum (const Intersect &isec, const Color &surface_color,
		 const LightModel &light_model, TraceState &tstate)
  const
{
  // First detect cases where the light isn't visible at all, by
  // examining the dot product of the surface normal with rays to two
  // opposite corners of the lights.
  //
  if (isec.normal.dot (pos - isec.point) < 0
      && isec.normal.dot (pos + side1 + side2 - isec.point) < 0)
    return Color (0, 0, 0);    
    
  // The light seems to be visible, so iterate over its surface in a
  // grid pattern, accumulating illumination from each "sublight".

  Color illum;

  // Distance between grid points in u and v directions.
  //
  dist_t u_step = 1.0 / JITTER_STEPS;
  dist_t v_step = 1.0 / JITTER_STEPS;

  // We scale down the brighness of each sublight so that in total they
  // add up to 1.
  //
  Color::component_t num_lights_scale = 1.0 / (JITTER_STEPS * JITTER_STEPS);

  // Iterate over the grid.
  //
  dist_t v_offs = 0;
  for (unsigned i = 0; i < JITTER_STEPS; i++)
    {
      dist_t u_offs = 0;
      for (unsigned j = 0; j < JITTER_STEPS; j++)
	{
	  // Compute position of sublight, as the current grid point
	  // plus a jitter factor
	  //
	  const Pos sublight_pos
	    = (pos
	       + side1 * (u_offs + random (u_step))
	       + side2 * (v_offs + random (v_step)));

	  const Ray sublight_ray (isec.point, sublight_pos);

	  const Vec &sublight_dir = sublight_ray.dir;
	  dist_t sublight_dist = sublight_ray.len;

	  // This expression is basically:
	  //
	  //   -cos (light_norm, sublight_dir))
	  //   / distance^2
	  //   / num_sublights
	  //
	  Color::component_t sublight_scale
	    = (num_lights_scale
	       * fabs (normal.dot (sublight_dir))
	       / (sublight_dist * sublight_dist));

	  // Cast shadow ray and compute surface color contribution from
	  // sublight.
	  //
	  illum += ray_illum (sublight_ray, color * sublight_scale, isec,
			      surface_color, light_model, tstate);

	  u_offs += u_step;    // step to next grid point in u direction
	}

      v_offs += v_step;	       // step to next grid point in v direction
    }

  return illum;
}

// Adjust this light's intensity by a factor of SCALE.
//
void
RectLight::scale_intensity (float scale)
{
  color *= scale;
}

// arch-tag: 60165b73-d34e-4f49-9a90-958daefdeb78
