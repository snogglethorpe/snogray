#ifndef __SCENE_H__
#define __SCENE_H__

#include <list>

#include "obj.h"
#include "intersect.h"

class Scene {
public:
  Intersect closest_intersect (const Ray &ray) const;
  bool intersects (const Ray &ray) const;
  
  void add (Obj *obj) { objs.push_back (obj); }

  Color render (const Intersect &isec) const;

private:
  std::list<Obj *> objs;
};

#endif /* __SCENE_H__ */

// arch-tag: 113d6236-471b-4184-92f5-9a03cf3a5221
