// point-light.cc -- Point light
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

#include "point-light.h"

using namespace Snogray;

// Generate (up to) NUM samples of this light and add them to SAMPLES.
// For best results, they should be distributed according to the light's
// intensity.
//
void
PointLight::gen_samples (const Intersect &isec, TraceState &tstate,
			 SampleRayVec &samples)
  const
{
  Vec lvec = pos - isec.point;

  if (isec.normal.dot (lvec) >= -Eps)
    {
      dist_t dist = lvec.length ();
      samples.add_light (color / (dist * dist), lvec.unit(), dist, this);
    }
}

// Modify the value of the BRDF samples in SAMPLES from FROM to TO,
// according to the light's intensity in the sample's direction.
//
void
PointLight::filter_samples (const Intersect &isec, TraceState &tstate,
			    SampleRayVec &samples,
			    SampleRayVec::iterator from,
			    SampleRayVec::iterator to)
  const
{
  // Unlikely to be much use (because very few samples will hit a point
  // light), but try...

  Vec lvec = pos - isec.point;
  dist_t dist = lvec.length ();

  for (SampleRayVec::iterator s = from; s != to; s++)
    if (dist < s->dist || s->dist == 0)
      {
	Vec samp_targ = s->dir * dist;

	dist_t dx = pos.x - samp_targ.x;
	dist_t dy = pos.y - samp_targ.y;
	dist_t dz = pos.z - samp_targ.z;

	if (dx < Eps && dx > -Eps
	    && dy < Eps && dy > -Eps
	    && dz < Eps && dz > -Eps)
	  s->set_light (color / (dist * dist), dist, this);
      }
}

// Adjust this light's intensity by a factor of SCALE.
//
void
PointLight::scale_intensity (float scale)
{
  color *= scale;
}

// arch-tag: 1ef7bd92-c1c5-4053-b4fb-f8a6bee1a1de
