// point-light.cc -- Point light
//
//  Copyright (C) 2005, 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
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
  Vec lvec = isec.normal_frame.to (pos);

  if (isec.cos_n (lvec) > 0 && isec.cos_geom_n (lvec) > 0)
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
