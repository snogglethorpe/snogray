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


Scene::Scene ()
  : horizon (DEFAULT_HORIZON), space (0), setup_done (false)
{ }

// The scene "owns" all its components, so frees them when it is destroyed
//
Scene::~Scene ()
{
  for (std::vector<const Light *>::const_iterator li = lights.begin();
       li != lights.end(); ++li)
    delete *li;

  for (std::vector<const Light::Sampler *>::const_iterator si
	 = light_samplers.begin();
       si != light_samplers.end(); ++si)
    delete *si;

  delete space;
}


// Object adding


// Add a surface.
//
void
Scene::add (const Surface *surface)
{
  ASSERT (! setup_done);

  if (space)
    {
      delete space;
      space = 0;
    }

  surfaces.add (surface);
}

// Add a light.
//
void
Scene::add (const Light *light)
{
  lights.push_back (light);
}

// Construct the search accelerator for this scene.
// SPACE_BUILDER_FACTORY says how to do it.
//
void
Scene::build_space (const SpaceBuilderFactory &space_builder_factory)
{
  if (! space)
    {
      UniquePtr<SpaceBuilder> space_builder
	(space_builder_factory.make_space_builder ());

      surfaces.add_to_space (*space_builder);

      space = space_builder->make_space ();
    }
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


// Scene::setup

// Do final setup for the scene.  This should be called after the scene
// is completely built, and nothing should be added after it it is
// called.
//
void
Scene::setup (const SpaceBuilderFactory &space_builder_factory)
{
  setup_done = true;

  // Make sure the space acceleration structures are built.
  //
  build_space (space_builder_factory);

  // Add area-lights.
  //
  surfaces.add_light_samplers (*this, light_samplers);

  // Create light-samplers for each light.
  //
  for (std::vector<const Light *>::iterator li = lights.begin ();
       li != lights.end (); ++li)
    (*li)->add_light_samplers (*this, light_samplers);

  // Record an abbreviated list of just environment-light samplers,
  // which we use when just returning the background.
  //
  for (std::vector<const Light::Sampler *>::const_iterator si
	 = light_samplers.begin();
       si != light_samplers.end(); ++si)
    if ((*si)->is_environ_light ())
      environ_light_samplers.push_back (*si);
}


// arch-tag: ecdd27ee-862e-436b-b0c6-357007955558
