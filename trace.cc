// trace.cc -- State during tracing
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "scene.h"
#include "illum.h"
#include "global-tstate.h"

#include "trace.h"

using namespace snogray;

// Constructor for root Trace
//
Trace::Trace (const Scene &_scene, GlobalTraceState &_global)
  : scene (_scene), parent (0), global (_global),
    type (SPONTANEOUS), origin (0), horizon_hint (0), depth (0), medium (0),
    _illum (0)
{
  _init ();
}

// Constructor for sub-traces
//
Trace::Trace (Type _type, Trace *_parent)
  : scene (_parent->scene), parent (_parent), global (_parent->global),
    type (_type), origin (0), horizon_hint (0), depth (_parent->depth + 1),
    medium (parent->medium),
    _illum (0)
{
  _init ();
}

void
Trace::_init ()
{
  unsigned num_lights = scene.num_lights ();

  shadow_hints = new const Surface*[num_lights];
  for (unsigned i = 0; i < num_lights; i++)
    shadow_hints[i] = 0;

  for (unsigned i = 0; i < NUM_TRACE_TYPES; i++)
    subtraces[i] = 0;
}

Trace::~Trace ()
{
  if (_illum)
    global.illum_global_state->put_illum (_illum);

  for (unsigned i = 0; i < NUM_TRACE_TYPES; i++)
    delete subtraces[i];

  delete[] shadow_hints;
}



// Searches back through the trace history to find the enclosing medium.
//
const Medium *
Trace::enclosing_medium ()
{
  const Trace *ts = this;

  int enclosure_level = 0;

  while (enclosure_level >= 0 && ts)
    {
      if (ts->type == REFRACTION_IN || ts->type == SHADOW_REFR_IN)
	enclosure_level--;
      else if (ts->type == REFRACTION_OUT || ts->type == SHADOW_REFR_OUT)
	enclosure_level++;

      ts = ts->parent;
    }

  return ts ? ts->medium : 0;
}



// Calculate the color perceived by looking along RAY.  This is the
// basic ray-tracing method.
//
Color
Trace::render (const Ray &ray)
{
  if (depth > global.params.max_depth)
    return scene.background (ray);

  Ray intersected_ray (ray, ray.t0 + global.params.min_trace, scene.horizon);

  const Surface::IsecInfo *isec_info = scene.intersect (intersected_ray, *this);
  if (isec_info)
    {
      Intersect isec = isec_info->make_intersect (intersected_ray, *this);

      // Calculate the appearance of the point on the surface we hit
      //
      Color radiance = isec.render ();

      // If we are looking through something other than air, attentuate
      // the surface appearance due to transmission through the current
      // medium.
      //
      if (medium)
	radiance = medium->attenuate (radiance, intersected_ray.t1);

      return radiance;
    }
  else
    return scene.background (ray);
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
Trace::shadow (const Ray &light_ray, const Color &light_color,
	       const Light &light)
{
  if (depth > global.params.max_depth * 2)
    //
    // We've exceeded the trace recursion limit, so guess a return value.
    // By returning LIGHT_COLOR, we're basically acting as if no more
    // shadowing objects occur between this point and the light.
    //
    // The other plausible return value, black, is _less_ likely to be
    // accurate, because true black shadows usually don't use this code path
    // (they should instead use the more efficient SHADOW_OPAQUE code).
    //
    return light_color;

  global.stats.surface_slow_shadow_traces++;

  Ray intersected_ray (light_ray,
		       light_ray.t0 + global.params.min_trace, light_ray.t1);

  const Surface::IsecInfo *isec_info = scene.intersect (intersected_ray, *this);
  if (isec_info)
    {
      Intersect isec = isec_info->make_intersect (intersected_ray, *this);

      // The distance traversed to hit CLOSEST.
      //
      dist_t dist = intersected_ray.length ();

      // Limited our continued search to the portion of INTERSECTED_RAY
      // which is past the closest intersection.
      //
      intersected_ray.t0 = intersected_ray.t1 + global.params.min_trace;
      intersected_ray.t1 = light_ray.t1;

      // Calculate the shadowing effect of the surface we hit
      //
      Color irradiance = isec.shadow (intersected_ray, light_color, light);

      // If we are looking through something other than air, attentuate
      // the surface appearance due to transmission through the current
      // medium.
      //
      if (medium)
	irradiance = medium->attenuate (irradiance, dist);

      return irradiance;
    }
  else
    return light_color;
}


// arch-tag: 03555891-462c-40bb-80b8-5f889c4cba44
