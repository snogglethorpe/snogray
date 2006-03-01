// trace.cc -- State during tracing
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "trace.h"

#include "scene.h"

using namespace Snogray;

// Constructor for root Trace
//
Trace::Trace (Scene &_scene, GlobalTraceState &_global)
  : scene (_scene), parent (0), global (_global),
    type (SPONTANEOUS), origin (0), horizon_hint (0), depth (0), medium (0)
{
  _init ();
}

// Constructor for sub-traces
//
Trace::Trace (TraceType _type, Trace *_parent)
  : scene (_parent->scene), parent (_parent), global (_parent->global),
    type (_type), origin (0), horizon_hint (0), depth (_parent->depth + 1),
    medium (parent->medium)
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
  if (depth > global.max_depth)
    return scene.background (ray);

  Ray intersected_ray (ray, scene.horizon);

  IsecParams isec_params;
  const Surface *closest
    = scene.intersect (intersected_ray, isec_params, *this);

  if (closest)
    {
      Intersect isec
	= closest->intersect_info (intersected_ray, isec_params, *this);

      // Calculate the appearance of the point on the surface we hit
      //
      Color radiance = isec.render ();

      // If we are looking through something other than air, attentuate
      // the surface appearance due to transmission through the current
      // medium.
      //
      if (medium)
	radiance = medium->attenuate (radiance, intersected_ray.len);

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
  if (depth > global.max_depth * 2)
    return scene.background (light_ray);

  Ray intersected_ray = light_ray;

  IsecParams isec_params;
  const Surface *closest
    = scene.intersect (intersected_ray, isec_params, *this);

  global.stats.surface_slow_shadow_traces++;

  if (closest)
    {
      Intersect isec
	= closest->intersect_info (intersected_ray, isec_params, *this);

      Ray continued_light_ray (intersected_ray.end(), intersected_ray.dir,
			       light_ray.len - intersected_ray.len);

      // Calculate the shadowing effect of the surface we hit
      //
      Color irradiance = isec.shadow (continued_light_ray, light_color, light);

      // If we are looking through something other than air, attentuate
      // the surface appearance due to transmission through the current
      // medium.
      //
      if (medium)
	irradiance = medium->attenuate (irradiance, intersected_ray.len);

      return irradiance;
    }
  else
    return light_color;
}


// arch-tag: 03555891-462c-40bb-80b8-5f889c4cba44
