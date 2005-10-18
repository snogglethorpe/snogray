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

#include <fstream>
#include <list>
#include <vector>

#include "obj.h"
#include "light.h"
#include "intersect.h"
#include "material.h"
#include "voxtree.h"
#include "trace-state.h"
#include "camera.h"

namespace Snogray {

class Scene
{
public:

  typedef std::vector<Light *>::const_iterator light_iterator_t;
  typedef std::list<Obj *>::const_iterator obj_iterator_t;
  typedef std::list<const Material *>::const_iterator material_iterator_t;

  static const unsigned DEFAULT_MAX_DEPTH = 5;
  static const unsigned DEFAULT_HORIZON = 10000;
  static const int DEFAULT_ASSUMED_GAMMA = 1;

  Scene ()
    : max_depth (DEFAULT_MAX_DEPTH), assumed_gamma (DEFAULT_ASSUMED_GAMMA)
  { }
  ~Scene ();

  // Calculate the color perceived by looking along RAY.  This is the
  // basic ray-tracing method.
  //
  Color render (const Ray &ray, TraceState &tstate)
    const
  {
    if (tstate.depth > max_depth)
      return background;

    Ray intersected_ray (ray, DEFAULT_HORIZON);

    const Obj *closest = intersect (intersected_ray, tstate);

    if (closest)
      {
	Intersect isec (intersected_ray, closest);

	// Calculate the appearance of the point on the object we hit
	//
	Color result = closest->material()->render (isec, tstate);

	// If we are looking through something other than air, attentuate
	// the surface appearance due to transmission through the current
	// medium.
	//
	if (tstate.medium)
	  result = tstate.medium->attenuate (result, intersected_ray.len);

	return result;
      }
    else
      return background;
  }

  // Shadow LIGHT_RAY, which points to a light with (apparent) color
  // LIGHT_COLOR. and return the shadow color.  This is basically like
  // the `render' method, but calls the material's `shadow' method
  // instead of it's `render' method.
  //
  // Note that this method is only used for `non-opaque' shadows --
  // opaque shadows (the most common kind) don't use it!
  //
  Color shadow (const Ray &light_ray, const Color &light_color,
		TraceState &tstate)
    const
  {
    if (tstate.depth > max_depth * 2)
      return background;

    Ray intersected_ray = light_ray;

    const Obj *closest = intersect (intersected_ray, tstate);

    stats.obj_slow_shadow_traces++;

    if (closest)
      {
	Ray continued_light_ray (intersected_ray.end(), intersected_ray.dir,
				 light_ray.len - intersected_ray.len);

	// Calculate the shadowing effect of the object we hit
	//
	Color result
	  = closest->material()->shadow (closest, continued_light_ray,
					 light_color, tstate);

	// If we are looking through something other than air, attentuate
	// the surface appearance due to transmission through the current
	// medium.
	//
	if (tstate.medium)
	  result = tstate.medium->attenuate (result, intersected_ray.len);

	return result;
      }
    else
      return light_color;
  }

  // Return the closest object in this scene which intersects the
  // bounded-ray RAY, or zero if there is none.  RAY's length is shortened
  // to reflect the point of intersection.
  //
  const Obj *intersect (Ray &ray, TraceState &tstate) const;

  // Return some object shadowing LIGHT_RAY from LIGHT, or 0 if there is
  // no shadowing object.  If an object it is returned, and it is _not_ an 
  // "opaque" object (shadow-type Material::SHADOW_OPAQUE), then it is
  // guaranteed there are no opaque objects casting a shadow.
  //
  // This is similar, but not identical to the behavior of the `intersect'
  // method -- `intersect' always returns the closest object and makes no
  // guarantees about the properties of further intersections.
  //
  const Obj *shadow_caster (const Ray &light_ray, Light &light,
			    TraceState &tstate)
    const;

  // Iterate over every light, calculating its contribution the color of
  // ISEC.  LIGHT_MODEL is used to calculate the actual effect; COLOR is
  // the "base color"
  //
  Color illum (const Intersect &isec, const Color &color,
	       const LightModel &light_model, TraceState &tstate)
    const;

  // Add various items to a scene.  All of the following "give" the
  // object to the scene -- freeing the scene will free them too.

  // Add an object
  //
  Obj *add (Obj *obj)
  {
    objs.push_back (obj);
    obj->add_to_space (obj_voxtree);
    return obj;
  }

  // Add a light
  Light *add (Light *light)
  {
    light->num = num_lights();	// Give LIGHT an index
    lights.push_back (light);
    return light;
  }

  // Add a material (we actually do nothing with these...)
  //
  const Material *add (const Material *mat)
  {
    materials.push_back (mat); return mat;
  }

  // Scene input
  //
  void load (const char *scene_file_name, const char *fmt, Camera &camera);
  void load (std::istream &stream, const char *fmt, Camera &camera);

  // Specific scene file formats
  //
  void load_aff_file (std::istream &stream, Camera &camera);

  unsigned num_lights () { return lights.size (); }

  void set_background (const Color &col) { background = col; }

  void set_assumed_gamma (float g) { assumed_gamma = g; }

  mutable struct Stats {
    Stats () : scene_intersect_calls (0),
	       obj_intersect_calls (0),
	       scene_shadow_tests (0),
	       shadow_hint_hits (0), shadow_hint_misses (0),
	       scene_slow_shadow_traces (0), obj_slow_shadow_traces (0),
	       horizon_hint_hits (0), horizon_hint_misses (0),
	       obj_intersects_tests (0)
    { }
    unsigned long long scene_intersect_calls;
    unsigned long long obj_intersect_calls;
    unsigned long long scene_shadow_tests;
    unsigned long long shadow_hint_hits;
    unsigned long long shadow_hint_misses;
    unsigned long long scene_slow_shadow_traces;
    unsigned long long obj_slow_shadow_traces;
    unsigned long long horizon_hint_hits;
    unsigned long long horizon_hint_misses;
    unsigned long long obj_intersects_tests;
    Voxtree::Stats voxtree_intersect;
    Voxtree::Stats voxtree_shadow;
  } stats;

  std::list<Obj *> objs;

  std::vector<Light *> lights;

  std::list<const Material *> materials;

  // Background color
  Color background;

  Voxtree obj_voxtree;

  unsigned max_depth;

  float assumed_gamma;
};

// These belong in "trace-state.h", but are here to avoid circular
// dependency problems.
//
inline Color
TraceState::render (const Ray &ray)
{
  return scene.render (ray, *this);
}

inline Color
TraceState::illum (const Intersect &isec, const Color &color,
		   const LightModel &light_model)
{
  return scene.illum (isec, color, light_model, *this);
}

inline Color
TraceState::shadow (const Ray &light_ray, const Color &light_color)
{
  return scene.shadow (light_ray, light_color, *this);
}

}

#endif /* __SCENE_H__ */

// arch-tag: 113d6236-471b-4184-92f5-9a03cf3a5221
