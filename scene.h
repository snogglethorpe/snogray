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
#include <vector>
#include <string>

#include "surface.h"
#include "light.h"
#include "intersect.h"
#include "material.h"
#include "trace.h"
#include "camera.h"
#include "envmap.h"
#include "shadow-ray.h"


namespace snogray {


class Space;


class Scene
{
public:

  static const unsigned DEFAULT_HORIZON = 1000000;
  static const int DEFAULT_ASSUMED_GAMMA = 1;

  Scene ();
  ~Scene ();

  // Returns the background color in the given direction.
  //
  Color background (const Vec &dir) const;
  Color background (const Ray &ray) const { return background (ray.dir); }

  // Return the closest surface in this scene which intersects the
  // bounded-ray RAY, or zero if there is none.  RAY's length is shortened
  // to reflect the point of intersection.
  //
  const Surface::IsecInfo *intersect (Ray &ray,
				      const Surface::IsecCtx &isec_ctx)
    const;

  // Return the strongest type of shadowing effect this scene has on
  // RAY.  If no shadow is cast, Material::SHADOW_NONE is returned;
  // otherwise if RAY is completely blocked, Material::SHADOW_OPAQUE is
  // returned; otherwise, Material::SHADOW_MEDIUM is returned.
  //
  Material::ShadowType shadow (const ShadowRay &ray, Trace &trace) const;

  // Add various items to a scene.  All of the following "give" the
  // surface to the scene -- freeing the scene will free them too.

  // Add a surface.
  //
  const Surface *add (const Surface *surface);

  // Add a light.
  //
  const Light *add (Light *light);

  // Add a material (we actually do nothing with these...)
  //
  const Material *add (const Material *mat);

  // Construct the search accelerator for this scene.
  // SPACE_BUILDER_BUILDER says how to do it.
  //
  void build_space (const SpaceBuilderBuilder *space_builder_builder);

  // Scene input
  //
  void load (const std::string &file_name, const std::string &fmt,
	     Camera &camera);

  // Specific scene file formats
  //
  void load_aff_file (const std::string &file_name, Camera &camera);

  unsigned num_surfaces () const { return surfaces.size (); }
  unsigned num_lights () const { return lights.size (); }

  void set_background (const Color &col);
  void set_background (const Ref<Envmap> &env_map);

  void set_light_map (const Ref<Envmap> &lmap);

  void set_assumed_gamma (float g) { assumed_gamma = g; }

  std::vector<const Surface *> surfaces;

  std::vector<const Light *> lights;

  std::vector<const Material *> materials;

  // A distance which is further than the furthest surface from any point.
  //
  dist_t horizon;

  // Background color or image
  //
  Color bg_color;
  Ref<Envmap> env_map;
  bool bg_set;			// true if background is non-default

  // Environment map used for lighting; currently this is only a
  // convenient storage place -- the actual light creation is done
  // outside the scene code.
  //
  Ref<Envmap> light_map;

  // Acceleration structure for doing ray-surface intersection testing.
  //
  const Space *space;

  float assumed_gamma;
};


// Convenience method calling Scene::shadow.
//
inline Material::ShadowType
Trace::shadow (const ShadowRay &ray)
{
  return scene.shadow (ray, *this);
}


}

#endif /* __SCENE_H__ */


// arch-tag: 113d6236-471b-4184-92f5-9a03cf3a5221
