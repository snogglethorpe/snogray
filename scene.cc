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
Scene::~Scene ()
{
  for (list<Obj *>::iterator oi = objs.begin(); oi != objs.end(); oi++)
    delete *oi;
  for (list<Light *>::iterator li = lights.begin(); li != lights.end(); li++)
    delete *li;
  for (list<Material *>::iterator mi = materials.begin(); mi != materials.end(); mi++)
    delete *mi;
}


// "Closest" intersection testing (tests all objects for intersection
// with a ray, returns the distance to the closest intersection)
//

struct SceneClosestIntersectCallback : Voxtree::FepiCallback
{
  SceneClosestIntersectCallback (const Ray &ray) : isec (ray), num_calls (0) { }

  virtual bool operator() (Obj *);

  Intersect isec;

  unsigned num_calls;
};

bool
SceneClosestIntersectCallback::operator () (Obj *obj)
{
  isec.update (obj);
  num_calls++;
  return true;
}

Intersect
Scene::closest_intersect (const Ray &ray)
{
  SceneClosestIntersectCallback closest_isec_cb (ray);

  stats.scene_closest_intersect_calls++;

  obj_voxtree.for_each_possible_intersector (ray, closest_isec_cb);

  closest_isec_cb.isec.finish ();

  stats.obj_closest_intersect_calls += closest_isec_cb.num_calls;

  return closest_isec_cb.isec;
}


// "Any" intersection testing (return true if any object intersects a ray)
//

struct SceneAnyIntersectCallback : Voxtree::FepiCallback
{
  SceneAnyIntersectCallback (const Ray &_ray, const Obj *_ignore = 0)
    : ray (_ray), ignore (_ignore), num_calls (0)
  { }

  virtual bool operator() (Obj *);

  const Ray &ray;
  const Obj *ignore;

  unsigned num_calls;
};

bool
SceneAnyIntersectCallback::operator () (Obj *obj)
{
  if (obj == ignore || obj->no_shadow)
    return true;
  num_calls++;
  return !obj->intersects (ray);
}

bool
Scene::intersects (const Ray &ray, const Obj *ignore)
{
  SceneAnyIntersectCallback any_isec_cb (ray, ignore);

  stats.scene_intersects_calls++;

  bool hit = !obj_voxtree.for_each_possible_intersector (ray, any_isec_cb);

  stats.obj_intersects_calls += any_isec_cb.num_calls;

  return hit;
}


// 

Color
Scene::render (const Intersect &isec)
{
  Color total_color;

  for (list<Light *>::const_iterator li = lights.begin();
       li != lights.end();
       li++)
    {
      Light *light = *li;
      Ray light_ray = Ray (isec.point, light->pos);

      if (! intersects (light_ray, isec.obj))
	total_color
	  += isec.render (light_ray.dir,
			  light->color / (light_ray.len * light_ray.len));
    }

  return total_color;
}

// arch-tag: ecdd27ee-862e-436b-b0c6-357007955558
