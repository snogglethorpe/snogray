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
  SceneClosestIntersectCallback (Ray &_ray,
				 TraceState &_tstate, const Obj *_origin = 0,
				 Voxtree::Stats *stats = 0)
    : IntersectCallback (stats), ray (_ray), origin (_origin),
      closest (0), tstate (_tstate), num_calls (0)
  { }

  virtual void operator() (Obj *);

  Ray &ray;
  const Obj *origin;

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
      if (obj->intersect (ray, origin))
	closest = obj;

      num_calls++;
    }
}

// Return the closest object in this scene which intersects the
// bounded-ray RAY, or zero if there is none.  RAY's length is shortened
// to reflect the point of intersection.  If ORIGIN is non-zero, then the
// _first_ intersection with that object is ignored (meaning that ORIGIN
// is totally ignored if it is flat).
//
const Obj *
Scene::intersect (Ray &ray, TraceState &tstate, const Obj *origin)
  const
{
  stats.scene_closest_intersect_calls++;

  // Make a callback, and call it for each object in the voxtree that may
  // intersect the ray.

  SceneClosestIntersectCallback
    closest_isec_cb (ray, tstate, origin, &stats.voxtree_closest_intersect);

  // If there's a horizon hint, try to use it to reduce the horizon before
  // searching -- voxtree searching can dramatically improve given a
  // limited search space.
  //
  if (tstate.horizon_hint)
    {
      if (tstate.horizon_hint->intersect (ray, origin))
	{
	  closest_isec_cb.closest = tstate.horizon_hint;
	  stats.horizon_hint_hits++;
	}
      else
	stats.horizon_hint_misses++;
    }

  obj_voxtree.for_each_possible_intersector (ray, closest_isec_cb);

  stats.obj_closest_intersect_calls += closest_isec_cb.num_calls;

  // Update the horizon hint to reflect what we found (0 if nothing).
  //
  tstate.horizon_hint = closest_isec_cb.closest;

  return closest_isec_cb.closest;
}


// Shadow intersection testing

struct SceneShadowedCallback : Voxtree::IntersectCallback
{
  SceneShadowedCallback (Light &_light, const Ray &_light_ray,
			 TraceState &_tstate, const Obj *_origin = 0,
			 Voxtree::Stats *stats = 0)
    : IntersectCallback (stats), 
      light (_light), light_ray (_light_ray), origin (_origin),
      shadowed (false), tstate (_tstate), num_tests (0)
  { }

  virtual void operator() (Obj *);

  Light &light;
  const Ray &light_ray;
  const Obj *origin;

  bool shadowed;

  TraceState &tstate;

  unsigned num_tests;
};

void
SceneShadowedCallback::operator () (Obj *obj)
{
  if (obj != origin && !obj->no_shadow)
    {
      num_tests++;

      shadowed = obj->intersects (light_ray);

      if (shadowed)
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

bool
Scene::shadowed (Light &light, const Ray &light_ray,
		 TraceState &tstate, const Obj *origin)
  const
{
  stats.scene_shadowed_tests++;

  // See if this light has a shadow hint (the last object that cast a shadow
  // from it); if it does, then try that object first, as it stands a better
  // chance of hitting than usual (because nearby points are often shadowed
  // from a given light by the same object).
  //
  const Obj *hint = tstate.shadow_hints[light.num];
  if (hint && hint != origin)
    {
      if (hint->intersects (light_ray))
	// It worked!  Return quickly.
	{
	  stats.shadow_hint_hits++;
	  return true;
	}
      else
	// It didn't work; clear this hint out.
	{
	  stats.shadow_hint_misses++;
	  tstate.shadow_hints[light.num] = 0;
	}
    }

  SceneShadowedCallback
    shadowed_cb (light, light_ray, tstate, origin, &stats.voxtree_shadowed);

  obj_voxtree.for_each_possible_intersector (light_ray, shadowed_cb);

  stats.obj_intersects_tests += shadowed_cb.num_tests;

  return shadowed_cb.shadowed;
}

// arch-tag: ecdd27ee-862e-436b-b0c6-357007955558
