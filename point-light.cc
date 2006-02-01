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

// Return the color as lit by this light of the surface at ISEC, with
// nominal color SURFACE_COLOR and reflectance function BRDF.
//
Color
PointLight::illum (const Intersect &isec, const Color &surface_color,
		   const Brdf &brdf, TraceState &tstate)
  const
{
  // If the dot-product of the impinging light-ray with the surface
  // normal is positive, that means the light is behind the surface,
  // so cannot light it ("self-shadowing"); otherwise, see if some
  // other surface casts a shadow.

  Vec light_vec = pos - isec.point;

  if (isec.normal.dot (light_vec) >= -Eps)
    {
      const Ray light_ray (isec.point, light_vec);

      return ray_illum (light_ray,
			color / (light_ray.len * light_ray.len),
			isec, surface_color, brdf, tstate);
    }
  else
    return Color (0, 0, 0);
}

// Adjust this light's intensity by a factor of SCALE.
//
void
PointLight::scale_intensity (float scale)
{
  color *= scale;
}

// arch-tag: 1ef7bd92-c1c5-4053-b4fb-f8a6bee1a1de
