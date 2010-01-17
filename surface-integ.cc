// surface-integ.cc -- Light integrator interface for surfaces
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "scene.h"

#include "surface-integ.h"

using namespace snogray;


// Return light from the scene arriving from the direction of RAY at its
// origin.  SAMPLE_NUM is the sample to use.
//
Tint
SurfaceInteg::li (const Ray &ray, unsigned sample_num)
{
  Ray intersected_ray (ray);
  const Surface::IsecInfo *isec_info
    = context.scene.intersect (intersected_ray, context);

  if (isec_info)
    {
      Trace trace (isec_info->ray, context);
      Intersect isec = isec_info->make_intersect (trace);
      return trace.medium.attenuate (lo (isec, sample_num), trace.ray.t1);
    }
  else
    return context.scene.background_with_alpha (ray);
}
