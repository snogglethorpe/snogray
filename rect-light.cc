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
#include "tripar-isec.h"

#include "rect-light.h"

using namespace Snogray;

// Generate (up to) NUM samples of this light and add them to SAMPLES.
// For best results, they should be distributed according to the light's
// intensity.
//
void
RectLight::gen_samples (const Intersect &isec, SampleRayVec &samples)
  const
{
  // Surface normal (of the surface intersected, not the light)
  //
  const Vec &N = isec.normal;

  // First detect cases where the light isn't visible at all, by
  // examining the dot product of the surface normal with rays to the
  // four corners of the light.
  //
  if (dot (N, pos - isec.point) > 0
      || dot (N, pos + side1 - isec.point) > 0
      || dot (N, pos + side2 - isec.point) > 0
      || dot (N, pos + side1 + side2 - isec.point) > 0)
    {    
      // The light seems to be visible, so iterate over its surface in a
      // grid pattern, adding a sample for each point

      // Distance between grid points in u and v directions.
      //
      dist_t u_step = 1.0 / JITTER_STEPS;
      dist_t v_step = 1.0 / JITTER_STEPS;

      // We scale down the brighness of each sample so that in total they
      // add up to 1.
      //
      Color::component_t num_samples_scale
	= 1.0 / (JITTER_STEPS * JITTER_STEPS);

      // Iterate over the grid.
      //
      dist_t v_offs = 0;
      for (unsigned i = 0; i < JITTER_STEPS; i++)
	{
	  dist_t u_offs = 0;
	  for (unsigned j = 0; j < JITTER_STEPS; j++)
	    {
	      // Compute position of sample, as the current grid point
	      // plus a jitter factor
	      //
	      const Pos sample_pos
		= (pos
		   + side1 * (u_offs + random (u_step))
		   + side2 * (v_offs + random (v_step)));

	      Vec lvec = sample_pos - isec.point;
	      dist_t dist = lvec.length ();

	      const Vec L = lvec.unit ();
	      float NL = dot (N, L);

	      if (NL > 0)
		{
		  // This expression is basically:
		  //
		  //   -cos (light_norm, lvec)) / distance^2 / num_samples
		  //
		  Color::component_t scale =
		    fabs (dot (normal, L)) * num_samples_scale / (dist * dist);

		  samples.add_light (power * scale, L, dist, this);
		}

	      u_offs += u_step;    // step to next grid point in u direction
	    }

	  v_offs += v_step;	       // step to next grid point in v direction
	}
    }
}

// Modify the value of the BRDF samples in SAMPLES from FROM to TO,
// according to the light's intensity in the sample's direction.
//
void
RectLight::filter_samples (const Intersect &isec, SampleRayVec &samples,
			   SampleRayVec::iterator from,
			   SampleRayVec::iterator to)
  const
{
  const Pos &org = isec.point;

  for (SampleRayVec::iterator s = from; s != to; s++)
    {
      dist_t u, v;
      dist_t dist
	= parallelogram_intersect (pos, side1, side2, org, s->dir, u, v);

      if (dist > 0 && (dist < s->dist || s->dist == 0))
	{
	  // This expression is basically:
	  //
	  //   -cos (light_norm, sublight_lvec)) / distance^2
	  //
	  Color::component_t scale = fabs (dot (normal, s->dir)) / (dist * dist);

	  s->set_light (power * scale, dist, this);
	}
    }
}

// Adjust this light's intensity by a factor of SCALE.
//
void
RectLight::scale_intensity (float scale)
{
  power *= scale;
}

// arch-tag: 60165b73-d34e-4f49-9a90-958daefdeb78
