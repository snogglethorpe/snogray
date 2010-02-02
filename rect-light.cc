// rect-light.cc -- Rectangular light
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "intersect.h"
#include "tripar-isec.h"
#include "scene.h"

#include "rect-light.h"


using namespace snogray;



// Return a sample of this light from the viewpoint of ISEC (using a
// surface-normal coordinate system, where the surface normal is
// (0,0,1)), based on the parameter PARAM.
//
Light::Sample
RectLight::sample (const Intersect &isec, const UV &param) const
{
  // The position and edges of the light, converted to the intersection
  // normal frame of reference.
  //
  Vec org = isec.normal_frame.to (pos);
  Vec s1 = isec.normal_frame.to (side1);
  Vec s2 = isec.normal_frame.to (side2);

  // First detect cases where the light isn't visible at all, by
  // examining the dot product of the surface normal with rays to the
  // four corners of the light.
  //
  if (isec.cos_n (org) >= 0
      || isec.cos_n (org + s1) >= 0
      || isec.cos_n (org + s2) >= 0
      || isec.cos_n (org + s1 + s2) >= 0)
    {
      // The light normal in the intersection normal frame of reference
      // (we actually use the negative of it in calculation below).
      //
      Vec neg_light_norm = -isec.normal_frame.to (normal);

      // Compute the position of the sample at U,V within the light.
      //
      const Vec s_vec = org + s1 * param.u + s2 * param.v;

      if (isec.cos_n (s_vec) > 0 && isec.cos_geom_n (s_vec) > 0)
	{
	  float dist = s_vec.length ();
	  float inv_dist = 1 / dist;
	  const Vec s_dir = s_vec * inv_dist;

	  // Area to solid-angle conversion, dw/dA
	  //   = cos (-light_normal, sample_dir) / distance^2
	  //
	  float dw_dA = dot (neg_light_norm, s_dir) * inv_dist * inv_dist;

	  if (dw_dA > Eps)
	    {
	      // Pdf Is (1 / Area) * (Dw/Da)
	      //
	      float pdf = 1 / (area * dw_dA);

	      return Sample (intensity, pdf, s_dir, dist);
	    }
	}
    }

  return Sample ();
}

// Evaluate this light in direction DIR from the viewpoint of ISEC (using
// a surface-normal coordinate system, where the surface normal is
// (0,0,1)).
//
Light::Value
RectLight::eval (const Intersect &isec, const Vec &dir) const
{
  // The light normal in the intersection normal frame of reference
  // (we actually use the negative of it in calculation below).
  //
  Vec neg_light_norm = -isec.normal_frame.to (normal);
  dist_t max_dist = isec.context.scene.horizon;

  Ray ray (isec.normal_frame.origin,
	   isec.normal_frame.from (dir),
	   max_dist);

  dist_t dist, u, v;
  if (parallelogram_intersect (pos, side1, side2, ray, dist, u, v))
    {
      // Area to solid-angle conversion, dw/dA
      //   = cos (-light_normal, sample_dir) / distance^2
      //
      float dw_dA = dot (neg_light_norm, dir) / (dist * dist);

      if (dw_dA > Eps)
	return Value (intensity, 1 / (area * dw_dA), dist);
    }

  return Value ();
}


// arch-tag: 60165b73-d34e-4f49-9a90-958daefdeb78
