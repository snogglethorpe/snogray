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
#include <list>

#include "intersect.h"
#include "excepts.h"

#include "phong.h"

using namespace Snogray;

// Source of "constant" (not-to-be-freed) Phong BRDFs
//
const Phong *
Snogray::phong (const Color &spec_col, float exp)
{
  static std::list<Phong> phongs;

  for (std::list<Phong>::iterator p = phongs.begin (); p != phongs.end (); p++)
    if (p->exponent == exp && p->specular_color == spec_col)
      return &(*p);

  phongs.push_front (Phong (spec_col, exp));

  return &phongs.front ();
}

// Generate (up to) NUM samples of this BRDF and add them to SAMPLES.
// For best results, they should be distributed according to the BRDF's
// importance function.
//
void
Phong::gen_samples (const Intersect &, SampleRayVec &)
  const
{
  throw std::runtime_error ("Phong::gen_samples");
}

// Modify the value of each of the light-samples in SAMPLES according to
// the BRDF's reflectivity in the sample's direction.
//
void
Phong::filter_samples (const Intersect &isec, SampleRayVec &,
		       SampleRayVec::iterator from,
		       SampleRayVec::iterator to)
  const
{
  const Vec &N = isec.normal;
  const Vec V = -isec.ray.dir;

  for (SampleRayVec::iterator s = from; s != to; s++)
    {
      const Vec &L = s->dir;
      float NL = dot (N, L);
      const Vec H = (V + L).unit ();
      float NH = dot (N, H);

      float specular = powf (NH, exponent);
      float diffuse = NL * M_1_PI; // standard lambertian diffuse term

      s->set_refl (isec.color * diffuse + specular_color * specular);
    }
}

// arch-tag: 11e5304d-111f-4597-a164-f08bd49e1563
