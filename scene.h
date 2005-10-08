// scene.h -- Scene description datatype
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SCENE_H__
#define __SCENE_H__

#include <list>

#include "obj.h"
#include "light.h"
#include "intersect.h"
#include "voxtree.h"

namespace Snogray {

class Scene
{
public:

  ~Scene ();

  Intersect closest_intersect (const Ray &ray);
  bool intersects (const Ray &ray, const Obj *ignore = 0);
  
  // Add various items to a scene.  All of the following "give" the
  // object to the scene -- freeing the scene will free them too.

  // Add an object
  Obj *add (Obj *obj)
  {
    objs.push_back (obj);
    obj_voxtree.add (obj);
    return obj;
  }

  Light *add (Light *light) { lights.push_back (light); return light; }

  Material *add (Material *mat) { materials.push_back (mat); return mat; }

  Color render (const Intersect &isec);
  Color render (const Ray  &ray)
  {
    Intersect isec = closest_intersect (ray);
    return isec.obj ? render (isec) : Color::black;
  }

  struct Stats {
    Stats () : scene_closest_intersect_calls (0),
	       obj_closest_intersect_calls (0),
	       scene_intersects_calls (0),
	       obj_intersects_calls (0)
    { }
    unsigned long long scene_closest_intersect_calls;
    unsigned long long obj_closest_intersect_calls;
    unsigned long long scene_intersects_calls;
    unsigned long long obj_intersects_calls;
    Voxtree::Stats voxtree_closest_intersect;
    Voxtree::Stats voxtree_intersects;
  } stats;

  std::list<Obj *> objs;
  std::list<Light *> lights;
  std::list<Material *> materials;

  Voxtree obj_voxtree;
};

}

#endif /* __SCENE_H__ */

// arch-tag: 113d6236-471b-4184-92f5-9a03cf3a5221
