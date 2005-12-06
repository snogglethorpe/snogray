// light.cc -- Light
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "color.h"
#include "ray.h"
#include "intersect.h"
#include "light-model.h"
#include "trace-state.h"
#include "scene.h"

#include "light.h"

using namespace Snogray;

Light::~Light () { } // stop gcc bitching

Color
Light::ray_illum (const Ray &light_ray, const Color &light_color,
		  const Intersect &isec, const Color &surface_color,
		  const LightModel &light_model, TraceState &tstate)
  const
{
  // Find any surface that's shadowing LIGHT_RAY.
  //
  const Surface *shadower = tstate.shadow_caster (light_ray, *this, isec);

  // If there's a shadowing surface, and it it is opaque, then we
  // need do nothing more...
  //
  if (shadower && shadower->shadow_type == Material::SHADOW_OPAQUE)
    return Color (0, 0, 0);
  else
    {
      // ... otherwise, we need to calculate exactly how much
      // light is received from LIGHT.

      Color illum_color = light_color / (light_ray.len * light_ray.len);

      // If there was actually some surface shadowing LIGHT_RAY,
      // it must be casting a partial shadow, so give it (and any
      // further surfaces) a chance to attentuate LIGHT_COLOR.
      //
      if (shadower)
	{
	  illum_color = tstate.shadow (light_ray, illum_color, *this);
	  tstate.scene.stats.scene_slow_shadow_traces++;
	}

      // Use the lighting model to calculate the resulting color
      // of the light-ray when viewed from our perspective.
      //
      return light_model.illum(isec, surface_color, light_ray.dir, illum_color);
    }
}

// arch-tag: 3915e032-063a-4bbf-aa37-c4bbaba9f8b1
