#include "scene.h"

using namespace std;

Intersect
Scene::closest_intersect (const Ray &ray)
{
  Intersect isec (ray);

  stats.scene_closest_intersect_calls++;

  for (list<Obj *>::const_iterator oi = objs.begin();
       oi != objs.end();
       oi++)
    {
      (*oi)->closest_intersect (isec);
      stats.obj_closest_intersect_calls++;
    }

  isec.finish ();

  return isec;
}

bool
Scene::intersects (const Ray &ray, const Obj *ignore)
{
  stats.scene_intersects_calls++;

  for (list<Obj *>::const_iterator oi = objs.begin();
       oi != objs.end();
       oi++)
    {
      Obj *obj = *oi;
      stats.obj_intersects_calls++;
      if (obj != ignore && obj->intersects (ray))
	return true;
    }

  return false;
}

Color
Scene::render (const Intersect &isec)
{
  Color total_color;
  Vec eye_dir = (isec.ray.origin - isec.point).unit ();

  for (list<Light *>::const_iterator li = lights.begin();
       li != lights.end();
       li++)
    {
      Light *light = *li;
      Ray light_ray = Ray (isec.point, light->pos);

      if (! intersects (light_ray, isec.obj))
	total_color
	  += isec.render (eye_dir, light_ray.dir,
			  light->color / (light_ray.len * light_ray.len));
    }

  return total_color;
}

// arch-tag: ecdd27ee-862e-436b-b0c6-357007955558
