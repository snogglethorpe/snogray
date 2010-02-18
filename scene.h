// scene.h -- Scene description datatype
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
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
#include "tint.h"
#include "intersect.h"
#include "material.h"
#include "media.h"
#include "space.h"
#include "shadow-ray.h"
#include "surface-group.h"


namespace snogray {


class Space;
class Envmap;
class Camera;


class Scene
{
public:

  static const unsigned DEFAULT_HORIZON = 1000000;

  Scene ();
  ~Scene ();

  // Returns the background color in the given direction.
  //
  Color background (const Vec &dir) const;
  Color background (const Ray &ray) const { return background (ray.dir); }

  // Return the background color, with the scene "background alpha" added.
  //
  Tint background_with_alpha (const Vec &dir) const
  {
    return Tint (background (dir), bg_alpha);
  }
  Tint background_with_alpha (const Ray &ray) const
  {
    return Tint (background (ray), bg_alpha);
  }

  // Return the closest surface in this scene which intersects the
  // bounded-ray RAY, or zero if there is none.  RAY's length is shortened
  // to reflect the point of intersection.
  //
  const Surface::IsecInfo *intersect (Ray &ray, RenderContext &context)
    const
  {
    context.stats.scene_intersect_calls++;
    return space->intersect (ray, context);
  }

  // Return true if any surface blocks RAY.
  //
  bool intersects (const ShadowRay &ray, RenderContext &context) const
  {
    context.stats.scene_shadow_tests++;
    return space->intersects (ray, context);
  }

  // Add various items to a scene.  All of the following "give" the
  // surface to the scene -- freeing the scene will free them too.

  // Add a surface.
  //
  void add (const Surface *surface);

  // Add a light.
  //
  void add (Light *light);

  // Construct the search accelerator for this scene.
  // SPACE_BUILDER_FACTORY says how to do it.
  //
  void build_space (const SpaceBuilderFactory *space_builder_factory);

  // Scene input
  //
  void load (const std::string &file_name, const std::string &fmt,
	     Camera &camera);

  unsigned num_surfaces () const { return surfaces.num_surfaces (); }
  unsigned num_lights () const { return lights.size (); }

  float background_alpha () const { return bg_alpha; }
  void set_background_alpha (float alpha) { bg_alpha = alpha; }

  // All surfaces in the scene.
  //
  SurfaceGroup surfaces;

  // All lights in the scene.
  //
  std::vector<Light *> lights;

  // "Environmental" lights in the scene.  This is a subset of LIGHTS.
  //
  std::vector<Light *> environ_lights;

  // Alpha value to use for background (either BG_COLOR or ENV_MAP).
  //
  float bg_alpha;

  // A distance which is further than the furthest surface from any point.
  //
  dist_t horizon;

  // Acceleration structure for doing ray-surface intersection testing.
  //
  const Space *space;
};


}

#endif /* __SCENE_H__ */


// arch-tag: 113d6236-471b-4184-92f5-9a03cf3a5221
