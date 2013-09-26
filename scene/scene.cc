// scene.cc -- Scene description datatype
//
//  Copyright (C) 2005-2010, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "util/snogassert.h"
#include "util/unique-ptr.h"
#include "space/space.h"
#include "space/space-builder.h"
#include "render/render-context.h"

#include "scene.h"


using namespace snogray;



// Do final setup for the scene.  This should be called after the scene
// is completely built, and nothing should be added after it it is
// called.
//
Scene::Scene (const Surface &_root_surface,
	      const SpaceBuilderFactory &space_builder_factory)
  : horizon (_root_surface.bbox ().diameter ()),
    root_surface (_root_surface),
    space (space_builder_factory.make_space (root_surface))
{
  // Add light-samplers for all lights.
  //
  root_surface.add_light_samplers (*this, light_samplers);

  // Record an abbreviated list of just environment-light samplers,
  // which we use when just returning the background.
  //
  for (std::vector<const Light::Sampler *>::const_iterator si
	 = light_samplers.begin();
       si != light_samplers.end(); ++si)
    if ((*si)->is_environ_light ())
      environ_light_samplers.push_back (*si);
}

Scene::~Scene ()
{
  // Delete light-samplers.
  //
  for (std::vector<const Light::Sampler *>::const_iterator si
 	 = light_samplers.begin();
       si != light_samplers.end(); ++si)
    delete *si;
}



// Scene background rendering

// Returns the background color in the given direction.
//
Color
Scene::background (const Vec &dir) const
{
  Color radiance = 0;

  for (std::vector<const Light::Sampler *>::const_iterator si
	 = environ_light_samplers.begin();
       si != environ_light_samplers.end(); ++si)
    radiance += (*si)->eval_environ (dir);

  return radiance;
}


// arch-tag: ecdd27ee-862e-436b-b0c6-357007955558
