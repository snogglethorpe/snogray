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
#include <vector>

#include "obj.h"
#include "light.h"
#include "intersect.h"
#include "material.h"
#include "voxtree.h"
#include "trace-state.h"

namespace Snogray {

class Scene
{
public:

  typedef std::vector<Light *>::const_iterator light_iterator_t;
  typedef std::list<Obj *>::const_iterator obj_iterator_t;
  typedef std::list<Material *>::const_iterator material_iterator_t;

  static const unsigned DEFAULT_MAX_DEPTH = 5;
  static const unsigned DEFAULT_HORIZON = 10000;
  static const int DEFAULT_ASSUMED_GAMMA = 1;

  Scene ()
    : max_depth (DEFAULT_MAX_DEPTH), assumed_gamma (DEFAULT_ASSUMED_GAMMA)
  { }
  ~Scene ();

  Intersect closest_intersect (const Ray &ray,
			       TraceState &tstate, const Obj *ignore = 0);
  bool shadowed (Light &light, const Ray &light_ray,
		 TraceState &tstate, const Obj *ignore = 0);
  
  // Add various items to a scene.  All of the following "give" the
  // object to the scene -- freeing the scene will free them too.

  // Add an object
  Obj *add (Obj *obj)
  {
    objs.push_back (obj);
    obj_voxtree.add (obj);
    return obj;
  }

  Light *add (Light *light)
  {
    light->num = num_lights();	// Give LIGHT an index
    lights.push_back (light);
    return light;
  }

  Material *add (Material *mat) { materials.push_back (mat); return mat; }

  unsigned num_lights () { return lights.size (); }

  void set_background (const Color &col) { background = col; }

  Color render (const Ray &ray, TraceState &tstate, const Obj *ignore = 0)
  {
    if (tstate.depth > max_depth)
      return background;

    Intersect isec = closest_intersect (ray, tstate, ignore);

    if (isec.obj)
      return isec.obj->material->render (isec, *this, tstate);
    else
      return background;
  }

  void set_assumed_gamma (float g) { assumed_gamma = g; }

  struct Stats {
    Stats () : scene_closest_intersect_calls (0),
	       obj_closest_intersect_calls (0),
	       scene_shadowed_tests (0),
	       shadow_hint_hits (0), shadow_hint_misses (0),
	       obj_intersects_tests (0)
    { }
    unsigned long long scene_closest_intersect_calls;
    unsigned long long obj_closest_intersect_calls;
    unsigned long long scene_shadowed_tests;
    unsigned long long shadow_hint_hits;
    unsigned long long shadow_hint_misses;
    unsigned long long obj_intersects_tests;
    Voxtree::Stats voxtree_closest_intersect;
    Voxtree::Stats voxtree_shadowed;
  } stats;

  std::list<Obj *> objs;

  std::vector<Light *> lights;

  std::list<Material *> materials;

  // Background color
  Color background;

  Voxtree obj_voxtree;

  unsigned max_depth;

  float assumed_gamma;
};

}

#endif /* __SCENE_H__ */

// arch-tag: 113d6236-471b-4184-92f5-9a03cf3a5221
