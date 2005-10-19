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

#include "point-light.h"

#include "ray.h"
#include "material.h"
#include "scene.h"
#include "trace-state.h"

using namespace Snogray;

// Return the color of the surface at ISEC, with nominal color
// SURFACE_COLOR, as lit by this light with lighting-model LIGHT_MODEL.
//
Color
PointLight::illum (const Intersect &isec, const Color &surface_color,
		   const LightModel &light_model, TraceState &tstate)
  const
{
  Ray light_ray (isec.point, pos);

  // Find any surface that's shadowing LIGHT_RAY.
  //
  const Surface *shadower = tstate.shadow_caster (light_ray, *this);

  // If there's a shadowing surface, and it it is opaque, then we
  // need do nothing more...
  //
  if (shadower && shadower->shadow_type == Material::SHADOW_OPAQUE)
    return Color (0, 0, 0);
  else
    {
      // ... otherwise, we need to calculate exactly how much
      // light is received from LIGHT.

      Color illum_color = color / (light_ray.len * light_ray.len);

      // If there was actually some surface shadowing LIGHT_RAY,
      // it must be casting a partial shadow, so give it (and any
      // further surfaces) a chance to attentuate LIGHT_COLOR.
      //
      if (shadower)
	{
	  illum_color = tstate.shadow (light_ray, illum_color);
	  tstate.scene.stats.scene_slow_shadow_traces++;
	}

      // Use the lighting model to calculate the resulting color
      // of the light-ray when viewed from our perspective.
      //
      return light_model.illum(isec, surface_color, light_ray.dir, illum_color);
    }
}

// Adjust this light's intensity by a factor of SCALE.
//
void
PointLight::scale_intensity (float scale)
{
  color *= scale;
}

// arch-tag: 1ef7bd92-c1c5-4053-b4fb-f8a6bee1a1de
