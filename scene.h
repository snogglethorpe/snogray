// scene.h -- Scene description datatype
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
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
#include "trace.h"
#include "camera.h"

namespace snogray {

class Envmap;

class Scene
{
public:

  typedef std::vector<const Light *>::const_iterator light_iterator_t;
  typedef std::list<Surface *>::const_iterator surface_iterator_t;
  typedef std::list<const Material *>::const_iterator material_iterator_t;

  static const unsigned DEFAULT_HORIZON = 1000000;
  static const int DEFAULT_ASSUMED_GAMMA = 1;

  Scene ()
    : horizon (DEFAULT_HORIZON), env_map (0), bg_set (false), light_map (0),
      assumed_gamma (DEFAULT_ASSUMED_GAMMA)
  { }
  ~Scene ();

  // Returns the background color in the direction pointed to by RAY
  //
  Color background (const Ray &ray) const;

  // Return the closest surface in this scene which intersects the
  // bounded-ray RAY, or zero if there is none.  RAY's length is shortened
  // to reflect the point of intersection.
  //
  const Surface *intersect (Ray &ray, IsecParams &isec_params, Trace &trace)
    const;

  // Return some surface shadowing LIGHT_RAY from LIGHT, or 0 if there
  // is no shadowing surface.  If a surface is returned, and it is _not_
  // an "opaque" surface (shadow-type Material::SHADOW_OPAQUE), then it
  // is guaranteed there are no opaque surfaces casting a shadow.
  //
  // ISEC is the intersection for which we are searching for shadow-casters.
  //
  // This is similar, but not identical to the behavior of the
  // `intersect' method -- `intersect' always returns the closest
  // surface and makes no guarantees about the properties of further
  // intersections.
  //
  const Surface *shadow_caster (const Ray &light_ray, const Intersect &isec,
				Trace &trace, const Light *light)
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

  unsigned num_surfaces () const { return surfaces.size (); }
  unsigned num_lights () const { return lights.size (); }

  void set_background (const Color &col);
  void set_background (const Envmap *env_map);

  void set_light_map (const Envmap *lmap);

  void set_assumed_gamma (float g) { assumed_gamma = g; }

  std::list<Surface *> surfaces;

  std::vector<const Light *> lights;

  std::list<const Material *> materials;

  // A distance which is further than the furthest surface from any point.
  //
  dist_t horizon;

  // Background color or image
  //
  Color bg_color;
  const Envmap *env_map;
  bool bg_set;			// true if background is non-default

  // Environment map used for lighting; currently this is only a
  // convenient storage place -- the actual light creation is done
  // outside the scene code.
  //
  const Envmap *light_map;

  Octree space;

  float assumed_gamma;
};

inline const Surface *
Trace::shadow_caster (const Ray &light_ray, const Intersect &isec,
		      const Light *light)
{
  return scene.shadow_caster (light_ray, isec, *this, light);
}

}

#endif /* __SCENE_H__ */

// arch-tag: 113d6236-471b-4184-92f5-9a03cf3a5221
