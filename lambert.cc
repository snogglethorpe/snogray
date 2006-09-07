// lambert.cc -- Lambertian reflectance function
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "intersect.h"
#include "excepts.h"

#include "lambert.h"

using namespace Snogray;

// There's only one possible lambert object so create it here.
//
const Lambert *Snogray::lambert = new Lambert;

// Generate (up to) NUM samples of this BRDF and add them to SAMPLES.
// For best results, they should be distributed according to the BRDF's
// importance function.
//
void
Lambert::gen_samples (const Intersect &, SampleRayVec &)
  const
{
  throw std::runtime_error ("Lambert::gen_samples");
}

// Modify the value of each of the light-samples in SAMPLES according to
// the BRDF's reflectivity in the sample's direction.
//
void
Lambert::filter_samples (const Intersect &isec, SampleRayVec &,
			 SampleRayVec::iterator from,
			 SampleRayVec::iterator to)
  const
{
  const Vec &N = isec.normal;

  for (SampleRayVec::iterator s = from; s != to; s++)
    if (s->val != 0)
      {
	const Vec &L = s->dir;
	float NL = dot (N, L);

	float diffuse = NL * M_1_PI; // standard lambertian diffuse term

	s->set_refl (isec.color * diffuse);
      }
}

// arch-tag: f61dbf3f-a5eb-4747-9bc5-18e793f35b6e
