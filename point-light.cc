// point-light.cc -- Point light
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
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

// Return the color of the surface at ISEC, with nominal color
// SURFACE_COLOR, as lit by this light with lighting-model LIGHT_MODEL.
//
Color
PointLight::illum (const Intersect &isec, const Color &surface_color,
		   const LightModel &light_model, TraceState &tstate)
  const
{
  return ray_illum (Ray (isec.point, pos), color,
		    isec, surface_color, light_model, tstate);
}

// Adjust this light's intensity by a factor of SCALE.
//
void
PointLight::scale_intensity (float scale)
{
  color *= scale;
}

// arch-tag: 1ef7bd92-c1c5-4053-b4fb-f8a6bee1a1de
