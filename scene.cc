// scene.cc -- Scene description datatype
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "global-tstate.h"
#include "octree.h"
#include "envmap.h"

#include "scene.h"


using namespace snogray;
using namespace std;


Scene::Scene ()
    : horizon (DEFAULT_HORIZON), env_map (0), bg_set (false), light_map (0),
      space (new Octree), assumed_gamma (DEFAULT_ASSUMED_GAMMA)
  { }

// The scene "owns" all its components, so frees them when it is destroyed
//
Scene::~Scene ()
{
  for (surface_iterator_t oi = surfaces.begin(); oi != surfaces.end(); oi++)
    delete *oi;
  for (light_iterator_t li = lights.begin(); li != lights.end(); li++)
    delete *li;
  for (material_iterator_t mi = materials.begin(); mi != materials.end(); mi++)
    delete *mi;

  delete env_map;

  if (light_map != env_map)
    delete light_map;

  delete space;
}


// Intersection testing

// Return the closest surface in this scene which intersects the
// bounded-ray RAY, or zero if there is none.  RAY's length is shortened
// to reflect the point of intersection.
//
const Surface::IsecInfo *
Scene::intersect (Ray &ray, const Surface::IsecCtx &isec_ctx) const
{
  Trace &trace = isec_ctx.trace;

  trace.global.stats.scene_intersect_calls++;

  // If there's a horizon hint, try to use it to reduce the horizon
  // before searching -- space searching can dramatically improve given
  // a limited search space.
  //
  const Surface::IsecInfo *hint_isec_info = 0;
  const Surface *hint = trace.horizon_hint;
  if (hint)
    {
      hint_isec_info = hint->intersect (ray, isec_ctx);

      if (hint_isec_info)
	trace.global.stats.horizon_hint_hits++;
      else
	{
	  trace.horizon_hint = 0; // clear the hint
	  trace.global.stats.horizon_hint_misses++;
	}
    }

  const Surface::IsecInfo *isec_info = space->intersect (ray, isec_ctx);

  // If the search didn't work out, use HINT_ISEC_INFO instead (which will
  // be zero if that didn't work out either).
  if (! isec_info)
    isec_info = hint_isec_info;

  return isec_info;
}

// Return the strongest type of shadowing effect this scene has on
// RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
// otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
// returned; otherwise, Material::SHADOW_MEDIUM is returned.
//
Material::ShadowType
Scene::shadow (const ShadowRay &ray, Trace &trace) const
{
  trace.global.stats.scene_shadow_tests++;

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
      const Surface *hint = trace.shadow_hints[ray.light->num];

      if (hint && hint != trace.origin)
	{
	  Material::ShadowType shadow_type = hint->shadow (ray);

	  if (shadow_type == Material::SHADOW_OPAQUE)
	    {
	      trace.global.stats.shadow_hint_hits++;
	      return shadow_type;
	    }
	  else
	    // It didn't work; clear this hint out.
	    {
	      trace.global.stats.shadow_hint_misses++;
	      trace.shadow_hints[ray.light->num] = 0;
	    }
	}
    }

  return space->shadow (ray, trace, ray.light);
}



// Returns the background color in the direction pointed to by RAY
//
Color
Scene::background (const Ray &ray) const
{
  if (env_map)
    {
      Color bg = env_map->map (ray.dir);

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
	      Color lmap_bg = light_map->map (ray.dir);

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
    {
      if (env_map != light_map)
	delete env_map;

      env_map = 0;
    }

  bg_color = col;
  bg_set = true;
}

void
Scene::set_background (const Envmap *map)
{
  if (env_map != light_map)
    delete env_map;

  env_map = map;
  bg_set = true;
}

void
Scene::set_light_map (const Envmap *lmap)
{
  if (light_map && light_map != env_map)
    delete light_map;

  light_map = lmap;
}


// arch-tag: ecdd27ee-862e-436b-b0c6-357007955558
