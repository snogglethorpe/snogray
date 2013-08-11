// point-light.cc -- Point light
//
//  Copyright (C) 2005-2008, 2010, 2012, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "intersect/intersect.h"
#include "geometry/cone-sample.h"

#include "point-light.h"


using namespace snogray;



// Return a sample of this light from the viewpoint of ISEC (using a
// surface-normal coordinate system, where the surface normal is
// (0,0,1)), based on the parameter PARAM.
//
Light::Sample
PointLight::sample (const Intersect &isec, const UV &) const
{
  // Vector from ISEC to the light position, in ISEC's normal frame.
  //
  Vec lvec = isec.normal_frame.to (frame.origin);

  if (isec.cos_n (lvec) > 0 && isec.cos_geom_n (lvec) > 0)
    {
      dist_t dist = lvec.length ();
      dist_t inv_dist = 1 / dist;

      // Cosine of the angle between the light-ray and the light axis.
      //
      float cos_dir
	= cos_angle ((isec.normal_frame.origin - frame.origin) * inv_dist,
		     frame.z);

      // If this is a spherical point-light (angle == 2*PI,
      // cos_half_angle == -1), then all directions are visible;
      // otherwise, we need to see if ISEC lies within the light's
      // cone.
      //
      if (cos_dir >= cos_half_angle)
	{
	  Color intens = intensity (cos_dir) * inv_dist * inv_dist;
	  Vec dir = lvec * inv_dist;
	  return Sample (intens, 1, dir, dist);
	}
    }

  return Sample ();
}

// Return a "free sample" of this light.
//
Light::FreeSample
PointLight::sample (const UV &, const UV &dir_param) const
{
  Vec dir = cone_sample (cos_half_angle, dir_param);
  float pdf = cone_sample_pdf (cos_half_angle);
  Color intens = intensity (dir.z); // DIR.z == cos_dir in this frame
  if (cos_half_angle != -1)
    dir = frame.from (dir);
  return FreeSample (intens, pdf, frame.origin, dir);
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
