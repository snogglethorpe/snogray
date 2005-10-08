#include "scene.h"

using namespace std;

Intersect
Scene::closest_intersect (const Ray &ray) const
{
  Intersect isec (ray);
  for (list<Obj>::const_iterator oi = objs.begin(); oi != objs.end(); oi++)
    oi->closest_intersect (isec);
  return isec;
}

bool
Scene::intersects (const Ray &ray) const
{
  for (list<Obj>::const_iterator oi = objs.begin(); oi != objs.end(); oi++)
    if (oi->intersects (ray))
      return true;
  return false;
}

// arch-tag: ecdd27ee-862e-436b-b0c6-357007955558
