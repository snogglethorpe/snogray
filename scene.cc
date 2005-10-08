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
  SceneClosestIntersectCallback (const Ray &ray, const Obj *_ignore = 0,
				 Voxtree::Stats *stats = 0)
    : IntersectCallback (stats), isec (ray), ignore (_ignore), num_calls (0)
  { }

  virtual void operator() (Obj *);

  Intersect isec;
  const Obj *ignore;

  unsigned num_calls;
};

void
SceneClosestIntersectCallback::operator () (Obj *obj)
{
  if (obj != ignore)
    {
      isec.update (obj);
      num_calls++;
    }
}

Intersect
Scene::closest_intersect (const Ray &ray, TraceState &tstate, const Obj *ignore)
{
  // XXX
  Ray bounded_ray (ray, 10000);

  SceneClosestIntersectCallback
    closest_isec_cb (bounded_ray, ignore, &stats.voxtree_closest_intersect);

  stats.scene_closest_intersect_calls++;

  obj_voxtree.for_each_possible_intersector (bounded_ray, closest_isec_cb);

  closest_isec_cb.isec.finish ();

  stats.obj_closest_intersect_calls += closest_isec_cb.num_calls;

  return closest_isec_cb.isec;
}


// Shadow intersection testing

struct SceneShadowedCallback : Voxtree::IntersectCallback
{
  SceneShadowedCallback (Light &_light, const Ray &_light_ray,
			 TraceState &_tstate, const Obj *_ignore = 0,
			 Voxtree::Stats *stats = 0)
    : IntersectCallback (stats), 
      light (_light), light_ray (_light_ray), ignore (_ignore),
      shadowed (false), tstate (_tstate), num_tests (0)
  { }

  virtual void operator() (Obj *);

  Light &light;
  const Ray &light_ray;
  const Obj *ignore;

  bool shadowed;

  TraceState &tstate;

  unsigned num_tests;
};

void
SceneShadowedCallback::operator () (Obj *obj)
{
  if (obj != ignore && !obj->no_shadow)
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
		 TraceState &tstate, const Obj *ignore)
{
  stats.scene_shadowed_tests++;

  // See if this light has a shadow hint (the last object that cast a shadow
  // from it); if it does, then try that object first, as it stands a better
  // chance of hitting than usual (because nearby points are often shadowed
  // from a given light by the same object).
  //
  Obj *hint = tstate.shadow_hints[light.num];
  if (hint && hint != ignore)
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
    shadowed_cb (light, light_ray, tstate, ignore, &stats.voxtree_shadowed);

  obj_voxtree.for_each_possible_intersector (light_ray, shadowed_cb);

  stats.obj_intersects_tests += shadowed_cb.num_tests;

  return shadowed_cb.shadowed;
}

// arch-tag: ecdd27ee-862e-436b-b0c6-357007955558
