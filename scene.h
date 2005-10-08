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

namespace Snogray {

class Scene
{
public:
  ~Scene ();

  Intersect closest_intersect (const Ray &ray);
  bool intersects (const Ray &ray, const Obj *ignore = 0);
  
  // All of the following "give" the object to the scene -- freeing the
  // scene will free them too.
  Obj *add (Obj *obj) { objs.push_back (obj); return obj; }
  Light *add (Light *light) { lights.push_back (light); return light; }
  // There's no point to the following, really, but it allows us to do
  // memory management on them maybe.
  Material *add (Material *mat) { materials.push_back (mat); return mat; }

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

  std::list<Obj *> objs;
  std::list<Light *> lights;
  std::list<Material *> materials;
};

}

#endif /* __SCENE_H__ */

// arch-tag: 113d6236-471b-4184-92f5-9a03cf3a5221
