#include "scene.h"

bool
Scene::closest_intersect (Intersect &isec)
{
  for (ObjRef objr = obj_list; objr; objr = objr->next)
    objr->obj->closest_intersect (isec);
  return 
}

bool
Scene::intersects (const Ray &ray)
{
  for (ObjRef objr = obj_list; objr; objr = objr->next)
    if (objr->obj->intersects (ray))
      return true;
  return false;
}

// arch-tag: ecdd27ee-862e-436b-b0c6-357007955558
