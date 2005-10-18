// scene.cc -- Scene description datatype
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "scene.h"

using namespace Snogray;
using namespace std;

// The scene "owns" all its components, so frees them when it is destroyed
//
Scene::~Scene ()
{
  for (obj_iterator_t oi = objs.begin(); oi != objs.end(); oi++)
    delete *oi;
  for (light_iterator_t li = lights.begin(); li != lights.end(); li++)
    delete *li;
  for (material_iterator_t mi = materials.begin(); mi != materials.end(); mi++)
    delete *mi;
}


// "Closest" intersection testing (tests all objects for intersection
// with a ray, returns the distance to the closest intersection)

struct SceneClosestIntersectCallback : Voxtree::IntersectCallback
{
  SceneClosestIntersectCallback (Ray &_ray, TraceState &_tstate, 
				 Voxtree::Stats *stats = 0)
    : IntersectCallback (stats), ray (_ray),
      closest (0), tstate (_tstate), num_calls (0)
  { }

  virtual void operator() (Obj *);

  Ray &ray;

  // The the closest intersecting object we've found
  //
  const Obj *closest;

  TraceState &tstate;

  unsigned num_calls;
};

void
SceneClosestIntersectCallback::operator () (Obj *obj)
{
  if (obj != tstate.horizon_hint)
    {
      if (obj->intersect (ray, tstate.origin_count (obj)))
	closest = obj;

      num_calls++;
    }
}

// Return the closest object in this scene which intersects the
// bounded-ray RAY, or zero if there is none.  RAY's length is shortened
// to reflect the point of intersection.
//
const Obj *
Scene::intersect (Ray &ray, TraceState &tstate)
  const
{
  stats.scene_intersect_calls++;

  // Make a callback, and call it for each object in the voxtree that may
  // intersect the ray.

  SceneClosestIntersectCallback
    closest_isec_cb (ray, tstate, &stats.voxtree_intersect);

  // If there's a horizon hint, try to use it to reduce the horizon before
  // searching -- voxtree searching can dramatically improve given a
  // limited search space.
  //
  const Obj *hint = tstate.horizon_hint;
  if (hint)
    {
      if (hint->intersect (ray, tstate.origin_count (hint)))
	{
	  closest_isec_cb.closest = hint;
	  stats.horizon_hint_hits++;
	}
      else
	stats.horizon_hint_misses++;
    }

  obj_voxtree.for_each_possible_intersector (ray, closest_isec_cb);

  stats.obj_intersect_calls += closest_isec_cb.num_calls;

  // Update the horizon hint to reflect what we found (0 if nothing).
  //
  tstate.horizon_hint = closest_isec_cb.closest;

  return closest_isec_cb.closest;
}


// Shadow intersection testing

struct SceneShadowCallback : Voxtree::IntersectCallback
{
  SceneShadowCallback (Light &_light, const Ray &_light_ray,
		       TraceState &_tstate, Voxtree::Stats *stats = 0)
    : IntersectCallback (stats), 
      light (_light), light_ray (_light_ray),
      shadower (0), tstate (_tstate), num_tests (0)
  { }

  virtual void operator() (Obj *);

  Light &light;
  const Ray &light_ray;

  // Shadowing object discovered.
  //
  const Obj *shadower;

  TraceState &tstate;

  unsigned num_tests;
};

void
SceneShadowCallback::operator () (Obj *obj)
{
  Material::ShadowType shadow_type = obj->shadow_type;

  if (obj != tstate.origin && shadow_type != Material::SHADOW_NONE)
    {
      num_tests++;

      if (obj->intersects (light_ray))
	{
	  shadower = obj;

	  if (shadow_type == Material::SHADOW_OPAQUE)
	    //
	    // A simple opaque object blocks everything; we can
	    // immediately return it.
	    {
	      // Remember which object cast a shadow from this light, so we
	      // can try it first next time.
	      //
	      tstate.shadow_hints[light.num] = obj;

	      // Stop looking any further.
	      //
	      stop_iteration ();
	    }
	}
    }
}

// Return some object shadowing LIGHT_RAY from LIGHT, or 0 if there is
// no shadowing object.  If an object it is returned, and it is _not_ an 
// "opaque" object (shadow-type Material::SHADOW_OPAQUE), then it is
// guaranteed there are no opaque objects casting a shadow.
//
// This is similar, but not identical to the behavior of the `intersect'
// method -- `intersect' always returns the closest object and makes no
// guarantees about the properties of further intersections.
//
const Obj *
Scene::shadow_caster (const Ray &light_ray, Light &light, TraceState &tstate)
  const
{
  stats.scene_shadow_tests++;

  // See if this light has a shadow hint (the last object that cast a shadow
  // from it); if it does, then try that object first, as it stands a better
  // chance of hitting than usual (because nearby points are often obscured
  // from a given light by the same object).
  //
  // Note that we only store hints for opaque objects, because only when we
  // find an opaque shadow caster can we immediately return (for non-opaque
  // shadow casters, we must keep looking in case there is also an opaque
  // shadow caster, or a closer non-opaque caster).
  //
  const Obj *hint = tstate.shadow_hints[light.num];
  if (hint && hint != tstate.origin)
    {
      if (hint->intersects (light_ray))
	{
	  stats.shadow_hint_hits++;
	  return hint;
	}
      else
	// It didn't work; clear this hint out.
	{
	  stats.shadow_hint_misses++;
	  tstate.shadow_hints[light.num] = 0;
	}
    }

  SceneShadowCallback
    shadow_cb (light, light_ray, tstate, &stats.voxtree_shadow);

  obj_voxtree.for_each_possible_intersector (light_ray, shadow_cb);

  stats.obj_intersects_tests += shadow_cb.num_tests;

  return shadow_cb.shadower;
}



// Iterate over every light, calculating its contribution the color of ISEC.
// LIGHT_MODEL is used to calculate the actual effect; COLOR is the "base color"
//
Color
Scene::illum (const Intersect &isec, const Color &color,
	      const LightModel &light_model, TraceState &tstate)
  const
{
  Color total_color;	// Accumulated colors from all light sources

  TraceState &shadow_tstate
    = tstate.subtrace_state (TraceState::SHADOW, isec.obj);

  for (light_iterator_t li = lights.begin(); li != lights.end(); li++)
    {
      Light *light = *li;
      Ray light_ray (isec.point, light->pos);

      // If the dot-product of the light-ray with the surface normal
      // is negative, that means the light is behind the surface, so
      // cannot light it ("self-shadowing"); otherwise, see if some
      // other object casts a shadow.

      if (isec.normal.dot (light_ray.dir) >= -Eps)
	{
	  // Find any object that's shadowing LIGHT_RAY.
	  //
	  const Obj *shadower = shadow_caster(light_ray, *light, shadow_tstate);

	  // If there's a shadowing object, and it it is opaque, then we
	  // need do nothing more...
	  //
	  if (!shadower || shadower->shadow_type == Material::SHADOW_MEDIUM)
	    {
	      // ... otherwise, we need to calculate exactly how much
	      // light is received from LIGHT.

	      Color light_color = light->color / (light_ray.len * light_ray.len);

	      // If there was actually some object shadowing LIGHT_RAY,
	      // it must be casting a partial shadow, so give it (and any
	      // further objects) a chance to attentuate LIGHT_COLOR.
	      //
	      if (shadower)
		{
		  light_color = shadow (light_ray, light_color, shadow_tstate);
		  stats.scene_slow_shadow_traces++;
		}

	      // Use the lighting model to calculate the resulting color
	      // of the light-ray when viewd from our perspective.
	      //
	      total_color
		+= light_model.illum (isec, color, light_ray.dir, light_color);
	    }
	}
    }

  return total_color;
}

// arch-tag: ecdd27ee-862e-436b-b0c6-357007955558
