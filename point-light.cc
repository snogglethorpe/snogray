// point-light.cc -- Point light
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "intersect.h"

#include "point-light.h"


using namespace snogray;


// Generate around NUM samples of this light and add them to SAMPLES.
// Return the actual number of samples (NUM is only a suggestion).
//
unsigned
PointLight::gen_samples (const Intersect &isec, unsigned,
			 IllumSampleVec &samples)
  const
{
  Vec lvec = pos - isec.pos;

  if (dot (isec.n, lvec) >= -Eps)
    {
      dist_t dist = lvec.length ();
      Vec s_dir = lvec / dist;
      samples.push_back (IllumSample (s_dir, color / (dist * dist), 0,
				      dist, this));
      return 1;
    }
  else
    return 0;
}


// arch-tag: 1ef7bd92-c1c5-4053-b4fb-f8a6bee1a1de
