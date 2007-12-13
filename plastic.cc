// plastic.h -- Plastic (thin, transmissive, reflective) material
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "fresnel.h"
#include "intersect.h"

#include "plastic.h"

using namespace snogray;

Color
Plastic::render (const Intersect &isec) const
{
  // Calculate fresnel surface reflection at the ray angle
  //
  float cos_xmit_angle = isec.cos_n (isec.v);
  float medium_ior = isec.trace.medium ? isec.trace.medium->ior : 1;
  float refl = Fresnel (medium_ior, ior).reflectance (cos_xmit_angle);

  Color radiance = 0;

  // Render transmitted light (some light is lost due to fresnel reflection
  // from the back surface).

  Color xmit = color * (1 - refl);

  if (xmit > Eps)
    {
      Ray xmit_ray (isec.normal_frame.origin, isec.ray.dir);
      radiance += xmit * isec.subtrace (Trace::TRANSMISSION).render (xmit_ray);
    }

  // Render reflected light

  if (refl > Eps)
    {
      Vec eye_dir = -isec.ray.dir;
      Vec mirror_dir = eye_dir.mirror (isec.normal_frame.z);
      Ray mirror_ray (isec.normal_frame.origin, mirror_dir);
      radiance += refl * isec.subtrace (Trace::REFLECTION).render (mirror_ray);
    }

  return radiance;
}

// Shadow LIGHT_RAY, which points to a light with (apparent) color
// LIGHT_COLOR. and return the shadow color.  This is basically like
// the `render' method, but calls the material's `shadow' method
// instead of its `render' method.
//
// Note that this method is only used for `non-opaque' shadows --
// opaque shadows (the most common kind) don't use it!
//
Color
Plastic::shadow (const Intersect &isec, const Ray &light_ray,
		 const Color &light_color, const Light &light)
  const
{
  // Calculate the amount of transmitted light which would be lost due
  // to Fresnel reflection from the back surface.
  //
  float cos_xmit_angle = dot (light_ray.dir, -isec.normal_frame.z);
  float medium_ior = isec.trace.medium ? isec.trace.medium->ior : 1;
  float refl = Fresnel (medium_ior, ior).reflectance (cos_xmit_angle);

  // Amount of light actually transmitted
  //
  Color xmited_light = light_color * color * (1 - refl);
      
  if (xmited_light > Eps)
    {
      Trace &sub_trace = isec.subtrace (Trace::SHADOW);
      return sub_trace.shadow (light_ray, xmited_light, light);
    }
  else
    return 0;
}

// arch-tag: cd843fe9-2c15-4212-80d7-7e302850c1a7
