// scene.h -- Scene description datatype
//
//  Copyright (C) 2005-2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_SCENE_H
#define SNOGRAY_SCENE_H

#include <vector>

#include "surface/surface.h"
#include "light/light.h"
#include "intersect.h"
#include "space/space.h"
#include "ray.h"
#include "surface/surface-group.h"


namespace snogray {


class SpaceBuilderFactory;


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

  // Return the closest surface in this scene which intersects the
  // bounded-ray RAY, or zero if there is none.  RAY's length is shortened
  // to reflect the point of intersection.
  //
  const Surface::IsecInfo *intersect (Ray &ray, RenderContext &context) const
  {
    context.stats.scene_intersect_calls++;
    return space->intersect (ray, context);
  }

  // Return true if any surface blocks RAY.
  //
  bool intersects (const Ray &ray, RenderContext &context) const
  {
    context.stats.scene_shadow_tests++;
    return space->intersects (ray, context);
  }

  // Return true if some surface in the scene completely occludes RAY.
  // If no surface completely occludes RAY, then return false, and
  // multiply TOTAL_TRANSMITTANCE by the transmittance of any surfaces
  // in this space which partially occlude RAY, evaluated in medium
  // MEDIUM.
  //
  // Note that this method does not try to handle non-trivial forms of
  // transparency/translucency (for instance, a "glass" material is
  // probably considered opaque because it changes light direction as
  // well as transmitting it), nor does it deal with anything except
  // surfaces.
  //
  bool occludes (const Ray &ray, const Medium &medium,
		 Color &total_transmittance,
		 RenderContext &context)
    const
  {
    context.stats.scene_shadow_tests++;
    return space->occludes (ray, medium, total_transmittance, context);
  }


  //
  // Add various items to a scene.  All of the following "give" the
  // surface to the scene -- freeing the scene will free them too.
  //

  // Add a surface.
  //
  void add (const Surface *surface);

  // Add a light.
  //
  void add (Light *light);

  // Construct the search accelerator for this scene.
  // SPACE_BUILDER_FACTORY says how to do it.
  //
  void build_space (const SpaceBuilderFactory &space_builder_factory);

  // Do final setup for the scene.  This should be called after the scene
  // is completely built, and nothing should be added after it it is
  // called.
  //
  void setup (const SpaceBuilderFactory &space_builder_factory);

  unsigned num_surfaces () const { return surfaces.num_surfaces (); }
  unsigned num_lights () const { return lights.size (); }

  // All surfaces in the scene.
  //
  SurfaceGroup surfaces;

  // All lights in the scene.
  //
  std::vector<Light *> lights;

  // "Environmental" lights in the scene.  This is a subset of LIGHTS.
  //
  std::vector<Light *> environ_lights;

  // A distance which is further than the furthest surface from any point.
  //
  dist_t horizon;

  // Acceleration structure for doing ray-surface intersection testing.
  //
  const Space *space;

  // True if Scene::setup has been called.  Nothing more can be added to
  // the scene after this is set.
  //
  bool setup_done;
};


}

#endif /* SNOGRAY_SCENE_H */


// arch-tag: 113d6236-471b-4184-92f5-9a03cf3a5221
