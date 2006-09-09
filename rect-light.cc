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

#include "rand.h"
#include "intersect.h"

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
  // First detect cases where the light isn't visible at all, by
  // examining the dot product of the surface normal with rays to the
  // four corners of the light.
  //
  if (dot (isec.n, pos - isec.pos) > 0
      || dot (isec.n, pos + side1 - isec.pos) > 0
      || dot (isec.n, pos + side2 - isec.pos) > 0
      || dot (isec.n, pos + side1 + side2 - isec.pos) > 0)
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

	      Vec lvec = sample_pos - isec.pos;
	      dist_t dist = lvec.length ();
	      dist_t inv_dist = 1 / dist;

	      const Vec l = lvec * inv_dist;
	      float nl = dot (isec.n, l);

	      if (nl > 0)
		{
		  // This expression is basically:
		  //
		  //   -cos (light_norm, lvec)) / distance^2 / num_samples
		  //
		  Color::component_t scale =
		    fabs (dot (normal, l))
		    * num_samples_scale * inv_dist * inv_dist;

		  samples.add_light (power * scale, l, dist, this);
		}

	      u_offs += u_step;    // step to next grid point in u direction
	    }

	  v_offs += v_step;	       // step to next grid point in v direction
	}
    }
}


// arch-tag: 60165b73-d34e-4f49-9a90-958daefdeb78
