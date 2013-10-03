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

#include "geometry/ray.h"
#include "surface/surface.h"
#include "light/light.h"
#include "light/light-sampler.h"
#include "render/intersect.h"
#include "space/space.h"


namespace snogray {


class SpaceBuilderFactory;


class Scene
{
public:

  Scene (const Surface &root_surface,
	 const SpaceBuilderFactory &space_builder_factory);
  ~Scene ();

  // Returns the background color in the given direction.
  //
  Color background (const Vec &dir) const;
  Color background (const Ray &ray) const { return background (ray.dir); }

  // Return the closest surface in this scene which intersects the
  // bounded-ray RAY, or zero if there is none.  RAY's length is shortened
  // to reflect the point of intersection.
  //
  const Surface::Renderable::IsecInfo *intersect (
					 Ray &ray,
					 RenderContext &context)
    const
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


  unsigned num_light_samplers () const { return light_samplers.size (); }

  // Return an axis-aligned bounding box containing the entire scene.
  //
  BBox bbox () const { return root_surface.bbox (); }

  // Light-samplers for all lights in the scene.
  //
  std::vector<const Light::Sampler *> light_samplers;

  // Light-samplers for "environmental" lights in the scene.  This is
  // a subset of LIGHT_SAMPLERS.
  //
  std::vector<const Light::Sampler *> environ_light_samplers;

  // A distance which is further than the furthest surface from any point.
  //
  dist_t horizon;


private:

  // All surfaces in the scene.
  //
  const Surface &root_surface;

  // Acceleration structure for doing ray-surface intersection testing.
  //
  UniquePtr<const Space> space;
};


}

#endif /* SNOGRAY_SCENE_H */


// arch-tag: 113d6236-471b-4184-92f5-9a03cf3a5221
