// scene.cc -- Scene description datatype
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "global-tstate.h"
#include "octree.h"
#include "envmap.h"

#include "scene.h"


using namespace snogray;
using namespace std;


Scene::Scene ()
    : horizon (DEFAULT_HORIZON), env_map (0), bg_set (false), light_map (0),
      space (new Octree), assumed_gamma (DEFAULT_ASSUMED_GAMMA)
  { }

// The scene "owns" all its components, so frees them when it is destroyed
//
Scene::~Scene ()
{
  for (surface_iterator_t oi = surfaces.begin(); oi != surfaces.end(); oi++)
    delete *oi;
  for (light_iterator_t li = lights.begin(); li != lights.end(); li++)
    delete *li;
  for (material_iterator_t mi = materials.begin(); mi != materials.end(); mi++)
    delete *mi;

  delete env_map;

  if (light_map != env_map)
    delete light_map;

  delete space;
}


// Intersection testing

// Return the closest surface in this scene which intersects the
// bounded-ray RAY, or zero if there is none.  RAY's length is shortened
// to reflect the point of intersection.
//
const Surface::IsecInfo *
Scene::intersect (Ray &ray, Trace &trace) const
{
  trace.global.stats.scene_intersect_calls++;
  return space->intersect (ray, trace);
}

// Return some surface shadowing LIGHT_RAY from LIGHT, or 0 if there is
// no shadowing surface.  If an surface it is returned, and it is _not_ an 
// "opaque" surface (shadow-type Material::SHADOW_OPAQUE), then it is
// guaranteed there are no opaque surfaces casting a shadow.
//
// ISEC is the intersection for which we are searching for shadow-casters.
//
// This is similar, but not identical to the behavior of the `intersect'
// method -- `intersect' always returns the closest surface and makes no
// guarantees about the properties of further intersections.
//
const Surface *
Scene::shadow_caster (const Ray &light_ray, const Intersect &isec,
		      Trace &trace, const Light *light)
  const
{
  trace.global.stats.scene_shadow_tests++;
  return space->shadow_caster (light_ray, isec, trace, light);
}



// Returns the background color in the direction pointed to by RAY
//
Color
Scene::background (const Ray &ray) const
{
  if (env_map)
    {
      Color bg = env_map->map (ray.dir);

      // If there are separate environment and light-maps, it's likely that
      // the latter is HDR and the former LDR, so try to add highlight
      // information to the environment-map using the HDR light-map.
      //
      // This is done by detecting a "saturated" LDR background color --
      // within epsilon of 1.0 -- and using the corresponding light-map
      // color instead if it is greater than 1.0.  This is done
      // separately for each color component.
      //
      // Essentially it switches to the low-res lightmap in very bright
      // areas; this can give much nicer reflections in some cases, and
      // the lower-res image tends to be unobjectional in such areas.
      //
      if (light_map && light_map != env_map)
	{
	  // "Saturation epsilon" -- if a color component is within this
	  // amount of 1.0, it is considered a possible "saturated" LDR
	  // pixel.
	  //
	  static const float SAT_EPS = 0.05;

	  // First we test the maximum component to see if it's possibly
	  // saturated; if not, there's no point in fetching the
	  // light-map color.
	  //
	  Color::component_t max = bg.max_component ();

	  if (max > 1-SAT_EPS && max < 1+SAT_EPS)
	    {
	      Color lmap_bg = light_map->map (ray.dir);

	      for (unsigned c = 0; c < Color::TUPLE_LEN; c++)
		if (bg[c] > 1-SAT_EPS && lmap_bg[c] > 1)
		  bg[c] = lmap_bg[c];
	    }
	}

      return bg;
    }
  else
    return bg_color;
}



void
Scene::set_background (const Color &col)
{
  if (env_map)
    {
      if (env_map != light_map)
	delete env_map;

      env_map = 0;
    }

  bg_color = col;
  bg_set = true;
}

void
Scene::set_background (const Envmap *map)
{
  if (env_map != light_map)
    delete env_map;

  env_map = map;
  bg_set = true;
}

void
Scene::set_light_map (const Envmap *lmap)
{
  if (light_map && light_map != env_map)
    delete light_map;

  light_map = lmap;
}


// arch-tag: ecdd27ee-862e-436b-b0c6-357007955558
