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
#include <string>

#include "surface.h"
#include "light.h"
#include "intersect.h"
#include "material.h"
#include "octree.h"
#include "trace-state.h"
#include "camera.h"

namespace Snogray {

class Cubetex;

class Scene
{
public:

  typedef std::vector<Light *>::const_iterator light_iterator_t;
  typedef std::list<Surface *>::const_iterator surface_iterator_t;
  typedef std::list<const Material *>::const_iterator material_iterator_t;

  static const unsigned DEFAULT_MAX_DEPTH = 5;
  static const unsigned DEFAULT_HORIZON = 10000;
  static const int DEFAULT_ASSUMED_GAMMA = 1;

  Scene ()
    : bg_cube (0),
      max_depth (DEFAULT_MAX_DEPTH),
      assumed_gamma (DEFAULT_ASSUMED_GAMMA)
  { }
  ~Scene ();

  // Calculate the color perceived by looking along RAY.  This is the
  // basic ray-tracing method.
  //
  Color render (const Ray &ray, TraceState &tstate)
    const
  {
    if (tstate.depth > max_depth)
      return background (ray);

    Ray intersected_ray (ray, DEFAULT_HORIZON);

    IsecParams isec_params;
    const Surface *closest = intersect (intersected_ray, isec_params, tstate);

    if (closest)
      {
	Intersect isec = closest->intersect_info (intersected_ray, isec_params);

	// Calculate the appearance of the point on the surface we hit
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
      return background (ray);
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
		const Light &light, TraceState &tstate)
    const
  {
    if (tstate.depth > max_depth * 2)
      return background (light_ray);

    Ray intersected_ray = light_ray;

    IsecParams isec_params;
    const Surface *closest = intersect (intersected_ray, isec_params, tstate);

    stats.surface_slow_shadow_traces++;

    if (closest)
      {
	Ray continued_light_ray (intersected_ray.end(), intersected_ray.dir,
				 light_ray.len - intersected_ray.len);

	// Calculate the shadowing effect of the surface we hit
	//
	Color result
	  = closest->material()->shadow (closest, continued_light_ray,
					 light_color, light, tstate);

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

  // Returns the background color in the direction pointed to by RAY
  //
  Color background (const Ray &ray) const;

  // Return the closest surface in this scene which intersects the
  // bounded-ray RAY, or zero if there is none.  RAY's length is shortened
  // to reflect the point of intersection.
  //
  const Surface *intersect (Ray &ray, IsecParams &isec_params,
			    TraceState &tstate)
    const;

  // Return some surface shadowing LIGHT_RAY from LIGHT, or 0 if there is
  // no shadowing surface.  If an surface it is returned, and it is _not_ an 
  // "opaque" surface (shadow-type Material::SHADOW_OPAQUE), then it is
  // guaranteed there are no opaque surfaces casting a shadow.
  //
  // ISEC is the intersection for which we are searching for shadow-casters.
  //
  // This is similar, but not identical to the behavior of the
  // `intersect' method -- `intersect' always returns the closest surface
  // and makes no guarantees about the properties of further intersections.
  //
  const Surface *shadow_caster (const Ray &light_ray, const Light &light,
				const Intersect &isec, TraceState &tstate)
    const;

  // Iterate over every light, calculating its contribution the color of
  // ISEC.  BRDF is used to calculate the actual effect; COLOR is
  // the "base color"
  //
  Color illum (const Intersect &isec, const Color &color,
	       const Brdf &brdf, TraceState &tstate)
    const;

  // Add various items to a scene.  All of the following "give" the
  // surface to the scene -- freeing the scene will free them too.

  // Add an surface
  //
  Surface *add (Surface *surface)
  {
    surfaces.push_back (surface);
    surface->add_to_space (space);
    return surface;
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
  void load (const std::string &file_name, const std::string &fmt,
	     Camera &camera);
  void load (std::istream &stream, const std::string &fmt, Camera &camera);

  // Specific scene file formats
  //
  void load_aff_file (std::istream &stream, Camera &camera);

  unsigned num_lights () { return lights.size (); }

  void set_background (const Color &col);
  void set_background (const Cubetex *cube);

  void set_assumed_gamma (float g) { assumed_gamma = g; }

  mutable struct Stats {
    Stats () : scene_intersect_calls (0),
	       surface_intersect_calls (0),
	       scene_shadow_tests (0),
	       shadow_hint_hits (0), shadow_hint_misses (0),
	       scene_slow_shadow_traces (0), surface_slow_shadow_traces (0),
	       horizon_hint_hits (0), horizon_hint_misses (0),
	       surface_intersects_tests (0),
	       illum_calls (0), illum_samples (0)
    { }
    unsigned long long scene_intersect_calls;
    unsigned long long surface_intersect_calls;
    unsigned long long scene_shadow_tests;
    unsigned long long shadow_hint_hits;
    unsigned long long shadow_hint_misses;
    unsigned long long scene_slow_shadow_traces;
    unsigned long long surface_slow_shadow_traces;
    unsigned long long horizon_hint_hits;
    unsigned long long horizon_hint_misses;
    unsigned long long surface_intersects_tests;
    unsigned long long illum_calls;
    unsigned long long illum_samples;
    Space::IsecStats space_intersect;
    Space::IsecStats space_shadow;
  } stats;

  std::list<Surface *> surfaces;

  std::vector<Light *> lights;

  std::list<const Material *> materials;

  // Background color
  Color bg_color;
  const Cubetex *bg_cube;

  Octree space;

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
TraceState::illum (const Intersect &isec, const Color &color, const Brdf &brdf)
{
  return scene.illum (isec, color, brdf, *this);
}

inline Color
TraceState::shadow (const Ray &light_ray, const Color &light_color,
		    const Light &light)
{
  return scene.shadow (light_ray, light_color, light, *this);
}

inline const Surface *
TraceState::shadow_caster (const Ray &light_ray, const Light &light,
			   const Intersect &isec)
{
  return scene.shadow_caster (light_ray, light, isec, *this);
}

}

#endif /* __SCENE_H__ */

// arch-tag: 113d6236-471b-4184-92f5-9a03cf3a5221
