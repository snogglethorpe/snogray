// scene.cc -- Scene description datatype
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

#include "space.h"
#include "envmap.h"
#include "snogassert.h"
#include "render-context.h"
#include "unique-ptr.h"

#include "scene.h"


using namespace snogray;


Scene::Scene ()
  : horizon (DEFAULT_HORIZON), space (0), setup_done (false)
{ }

// The scene "owns" all its components, so frees them when it is destroyed
//
Scene::~Scene ()
{
  for (std::vector<Light *>::const_iterator li = lights.begin();
       li != lights.end(); ++li)
    delete *li;

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
Scene::add (Light *light)
{
  lights.push_back (light);

  if (light->is_environ_light ())
    environ_lights.push_back (light);
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

  for (std::vector<Light *>::const_iterator li = environ_lights.begin();
       li != environ_lights.end(); ++li)
    radiance += (*li)->eval_environ (dir);

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
  surfaces.add_lights (lights);

  // call each light's Light::scene_setup method.
  //
  for (std::vector<Light *>::iterator li = lights.begin ();
       li != lights.end (); ++li)
    (*li)->scene_setup (*this);
}


// arch-tag: ecdd27ee-862e-436b-b0c6-357007955558
