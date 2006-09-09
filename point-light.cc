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

#include "intersect.h"

#include "point-light.h"

using namespace Snogray;

// Generate (up to) NUM samples of this light and add them to SAMPLES.
// For best results, they should be distributed according to the light's
// intensity.
//
void
PointLight::gen_samples (const Intersect &isec, SampleRayVec &samples)
  const
{
  Vec lvec = pos - isec.pos;

  if (dot (isec.n, lvec) >= -Eps)
    {
      dist_t dist = lvec.length ();
      samples.add_light (color / (dist * dist), lvec.unit(), dist, this);
    }
}


// arch-tag: 1ef7bd92-c1c5-4053-b4fb-f8a6bee1a1de
