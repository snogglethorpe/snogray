// scene.cc -- Scene description datatype
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <memory>		// for auto_ptr

#include "space.h"
#include "envmap.h"
#include "trace-context.h"
#include "trace-cache.h"

#include "scene.h"


using namespace snogray;
using namespace std;


Scene::Scene ()
  : horizon (DEFAULT_HORIZON),
    bg_color (0), env_map (0), bg_set (false), light_map (0), space (0)
{ }

// The scene "owns" all its components, so frees them when it is destroyed
//
Scene::~Scene ()
{
  for (std::vector<const Light *>::const_iterator li = lights.begin();
       li != lights.end(); ++li)
    delete *li;

  delete space;
}


// Object adding


// Add a surface.
//
void
Scene::add (const Surface *surface)
{
  if (space)
    {
      delete space;
      space = 0;
    }

  surfaces.add (surface);
}

// Add a light.
//
void
Scene::add (Light *light)
{
  light->num = num_lights();	// Give LIGHT an index
  lights.push_back (light);
}

// Construct the search accelerator for this scene.
// SPACE_BUILDER_BUILDER says how to do it.
//
void
Scene::build_space (const SpaceBuilderBuilder *space_builder_builder)
{
  if (! space)
    {
      std::auto_ptr<SpaceBuilder> space_builder
	(space_builder_builder->make_space_builder ());

      surfaces.add_to_space (*space_builder);

      space = space_builder->make_space ();
    }
}


// Intersection testing

// Return the closest surface in this scene which intersects the
// bounded-ray RAY, or zero if there is none.  RAY's length is shortened
// to reflect the point of intersection.
//
const Surface::IsecInfo *
Scene::intersect (Ray &ray, const IsecCtx &isec_ctx) const
{
  TraceContext &context = isec_ctx.context;
  TraceCache &cache = isec_ctx.cache;

  context.stats.scene_intersect_calls++;

  // If there's a horizon hint, try to use it to reduce the horizon
  // before searching -- space searching can dramatically improve given
  // a limited search space.
  //
  const Surface::IsecInfo *hint_isec_info = 0;
  const Surface *hint = cache.horizon_hint;
  if (hint)
    {
      hint_isec_info = hint->intersect (ray, isec_ctx);

      context.stats.intersect.surface_intersects_tests++;

      if (hint_isec_info)
	context.stats.horizon_hint_hits++;
      else
	{
	  cache.horizon_hint = 0; // clear the hint
	  context.stats.horizon_hint_misses++;
	}
    }

  const Surface::IsecInfo *isec_info = space->intersect (ray, isec_ctx);

  // If the search worked (ISEC_INFO is non-zero), update the horizon hint
  // to reflect the new intersection, otherwise, use HINT_ISEC_INFO instead
  // (which will be zero if that didn't work out either).
  //
  if (isec_info)
    cache.horizon_hint = isec_info->outermost_surface ();
  else
    isec_info = hint_isec_info;

  return isec_info;
}

// Return the strongest type of shadowing effect this scene has on
// RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
// otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
// returned; otherwise, Material::SHADOW_MEDIUM is returned.
//
Material::ShadowType
Scene::shadow (const ShadowRay &ray, IsecCtx &isec_ctx) const
{
  TraceContext &context = isec_ctx.context;
  TraceCache &cache = isec_ctx.cache;

  context.stats.scene_shadow_tests++;

  // See if this light has a shadow hint (the last surface that cast a
  // shadow from it); if it does, then try that surface first, as it
  // stands a better chance of hitting than usual (because nearby points
  // are often obscured from a given light by the same surface).
  //
  // Note that in the case where the hint refers to non-opaque surface,
  // we will return it immediately, just like an opaque surface.  This
  // will not cause errors, because the shadow-tracing "slow path" (which
  // will get used if a non-opaque surface is returned) still does the
  // right thing in this case, simply more slowly; in the case where a
  // new opaque surface is found, the hint will be updated elsewhere (in
  // Material::shadow actually).
  //
  if (ray.light)
    {
      const Surface *hint = cache.shadow_hints[ray.light->num];

      if (hint)
	{
	  Material::ShadowType shadow_type = hint->shadow (ray, isec_ctx);

	  if (shadow_type == Material::SHADOW_OPAQUE)
	    {
	      context.stats.shadow_hint_hits++;
	      return shadow_type;
	    }
	  else
	    // It didn't work; clear this hint out.
	    {
	      context.stats.shadow_hint_misses++;
	      cache.shadow_hints[ray.light->num] = 0;
	    }
	}
    }

  return space->shadow (ray, isec_ctx, ray.light);
}

// Return true if any surface blocks RAY.  This is the fastest
// intersection method, because it can return as soon as it it finds any
// intersection.
//
bool
Scene::shadows (const ShadowRay &ray, IsecCtx &isec_ctx) const
{
  TraceContext &context = isec_ctx.context;
  TraceCache &cache = isec_ctx.cache;

  context.stats.scene_shadow_tests++;

  // See if this light has a shadow hint (the last surface that cast a
  // shadow from it); if it does, then try that surface first, as it
  // stands a better chance of hitting than usual (because nearby points
  // are often obscured from a given light by the same surface).
  //
  // Note that in the case where the hint refers to non-opaque surface,
  // we will return it immediately, just like an opaque surface.  This
  // will not cause errors, because the shadow-tracing "slow path" (which
  // will get used if a non-opaque surface is returned) still does the
  // right thing in this case, simply more slowly; in the case where a
  // new opaque surface is found, the hint will be updated elsewhere (in
  // Material::shadow actually).
  //
  if (ray.light)
    {
      const Surface *hint = cache.shadow_hints[ray.light->num];

      if (hint)
	{
	  Material::ShadowType shadow_type = hint->shadow (ray, isec_ctx);

	  if (shadow_type != Material::SHADOW_NONE)
	    {
	      context.stats.shadow_hint_hits++;
	      return shadow_type;
	    }
	  else
	    // It didn't work; clear this hint out.
	    {
	      context.stats.shadow_hint_misses++;
	      cache.shadow_hints[ray.light->num] = 0;
	    }
	}
    }

  return space->shadows (ray, isec_ctx, ray.light);
}


// Scene background rendering

// Returns the background color in the given direction.
//
Color
Scene::background (const Vec &dir) const
{
  if (env_map)
    {
      Color bg = env_map->map (dir);

      // If there are separate environment and light-maps, it's likely that
      // the latter is HDR and the former LDR, so try to add highlight
      // information to the environment-map using the HDR light-map.
      //
      // This is done by detecting a "saturated" LDR background color --
      // within epsilon of 1.0 -- and using the corresponding light-map
      // color instead if it is greater than 1.0.  This is done
      // separately for each color component.
      //
      // Essentially it switches to the low-res lightmap in very bright
      // areas; this can give much nicer reflections in some cases, and
      // the lower-res image tends to be unobjectional in such areas.
      //
      if (light_map && light_map != env_map)
	{
	  // "Saturation epsilon" -- if a color component is within this
	  // amount of 1.0, it is considered a possible "saturated" LDR
	  // pixel.
	  //
	  static const float SAT_EPS = 0.05;

	  // First we test the maximum component to see if it's possibly
	  // saturated; if not, there's no point in fetching the
	  // light-map color.
	  //
	  Color::component_t max = bg.max_component ();

	  if (max > 1-SAT_EPS && max < 1+SAT_EPS)
	    {
	      Color lmap_bg = light_map->map (dir);

	      for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
		if (bg[c] > 1-SAT_EPS && lmap_bg[c] > 1)
		  bg[c] = lmap_bg[c];
	    }
	}

      return bg;
    }
  else
    return bg_color;
}



void
Scene::set_background (const Color &col)
{
  if (env_map)
    env_map = 0;

  bg_color = col;
  bg_set = true;
}

void
Scene::set_background (const Ref<Envmap> &map)
{
  env_map = map;
  bg_set = true;
}

void
Scene::set_light_map (const Ref<Envmap> &lmap)
{
  light_map = lmap;
}


// arch-tag: ecdd27ee-862e-436b-b0c6-357007955558
