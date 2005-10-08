#include "scene.h"

using namespace std;

Intersect
Scene::closest_intersect (const Ray &ray) const
{
  Intersect isec (ray);

  for (list<Obj *>::const_iterator oi = objs.begin(); oi != objs.end(); oi++)
    oi->closest_intersect (isec);

  isec.finish ();

  return isec;
}

bool
Scene::intersects (const Ray &ray, Space::dist_t max_dist_squared) const
{
  for (list<Obj>::const_iterator oi = objs.begin(); oi != objs.end(); oi++)
    if (oi->intersects (ray, max_dist_squared))
      return true;

  return false;
}

Color
Scene::render (const Intersect &isec) const
{
  Color total_color;

  for (list<Light *>::const_iterator li = lights.begin();
       li != lights.end();
       li++)
    {
      Light *light = *li;
      Vec light_vec = light->pos - isec.point;
      Ray light_ray = Ray (isec.point, light->pos);
      Space::dist_t light_dist_sq = light_vec.length_squared ();

      if (! intersects (light_ray, light_dist_sq))
	total_color += isec.render (light->color / light_dist_sq, light_vec);
    }

  return total_color;
}

// arch-tag: ecdd27ee-862e-436b-b0c6-357007955558
