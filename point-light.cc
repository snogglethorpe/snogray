// point-light.cc -- Point light
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2010  Miles Bader <miles@gnu.org>
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



// Return a sample of this light from the viewpoint of ISEC (using a
// surface-normal coordinate system, where the surface normal is
// (0,0,1)), based on the parameter PARAM.
//
Light::Sample
PointLight::sample (const Intersect &isec, const UV &) const
{
  Vec lvec = isec.normal_frame.to (pos);

  if (isec.cos_n (lvec) > 0 && isec.cos_geom_n (lvec) > 0)
    {
      dist_t dist = lvec.length ();
      Vec s_dir = lvec / dist;
      return Sample (dist * dist, 1, s_dir, dist);
    }

  return Sample ();
}

// Evaluate this light in direction DIR from the viewpoint of ISEC (using
// a surface-normal coordinate system, where the surface normal is
// (0,0,1)).
//
Light::Value
PointLight::eval (const Intersect &, const Vec &) const
{
  return Value ();  // DIR will always fail to point exactly to th
}


// arch-tag: 1ef7bd92-c1c5-4053-b4fb-f8a6bee1a1de
