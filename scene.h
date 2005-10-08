#ifndef __SCENE_H__
#define __SCENE_H__

#include <list>

#include "obj.h"
#include "light.h"
#include "intersect.h"

class Scene {
public:
  Intersect closest_intersect (const Ray &ray);
  bool intersects (const Ray &ray, const Obj *ignore = 0);
  
  void add (Obj *obj) { objs.push_back (obj); }
  void add (Light *light) { lights.push_back (light); }

  Color render (const Intersect &isec);

  struct Stats {
    Stats () : scene_closest_intersect_calls (0),
	       obj_closest_intersect_calls (0),
	       scene_intersects_calls (0),
	       obj_intersects_calls (0)
    { }
    unsigned scene_closest_intersect_calls;
    unsigned obj_closest_intersect_calls;
    unsigned scene_intersects_calls;
    unsigned obj_intersects_calls;
  } stats;

private:
  std::list<Obj *> objs;
  std::list<Light *> lights;
};

#endif /* __SCENE_H__ */

// arch-tag: 113d6236-471b-4184-92f5-9a03cf3a5221
