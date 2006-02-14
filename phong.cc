// phog.cc -- Phong reflectance function
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cmath>

#include "intersect.h"
#include "excepts.h"

#include "phong.h"

using namespace Snogray;

// Generate (up to) NUM samples of this BRDF and add them to SAMPLES.
// For best results, they should be distributed according to the BRDF's
// importance function.
//
void
Phong::gen_samples (const Intersect &isec, const Color &color,
		    TraceState &tstate, SampleRayVec &samples)
  const
{
  throw std::runtime_error ("Phong::gen_samples");
}

// Modify the value of each of the light-samples in SAMPLES according to
// the BRDF's reflectivity in the sample's direction.
//
void
Phong::filter_samples (const Intersect &isec, const Color &color,
		       TraceState &tstate, SampleRayVec &samples,
		       SampleRayVec::iterator from,
		       SampleRayVec::iterator to)
  const
{
  for (SampleRayVec::iterator s = from; s != to; s++)
    {
      float diffuse_component = isec.normal.dot (s->dir);

      if (diffuse_component > 0)
	{
	  float specular_component
	    = powf (isec.normal.dot ((s->dir - isec.ray.dir).unit()), exponent);

	  Color total_color = color * diffuse_component;

	  total_color += specular_color * specular_component;

	  s->set_refl (total_color);
	}
      else
	s->invalidate ();
    }
}

// arch-tag: 11e5304d-111f-4597-a164-f08bd49e1563
