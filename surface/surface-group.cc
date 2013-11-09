// surface-group.cc -- Group of surfaces
//
//  Copyright (C) 2007, 2008, 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "space/space-builder.h"

#include "surface-group.h"


using namespace snogray;


SurfaceGroup::~SurfaceGroup ()
{
  for (std::vector<Surface *>::const_iterator si = surfaces.begin();
       si != surfaces.end(); ++si)
    delete *si;

  for (std::vector<Light *>::const_iterator li = lights.begin();
       li != lights.end(); ++li)
    delete *li;
}



// Add SURFACE to this group.
//
void
SurfaceGroup::add (Surface *surface)
{
  surfaces.push_back (surface);
  _bbox += surface->bbox ();
}

// Add LIGHT to this group.
//
void
SurfaceGroup::add (Light *light)
{
  lights.push_back (light);
}


// Transform the geometry of this surface by XFORM.
//
void
SurfaceGroup::transform (const Xform &xform)
{
  for (std::vector<Surface *>::const_iterator si = surfaces.begin();
       si != surfaces.end(); ++si)
    (*si)->transform (xform);

  for (std::vector<Light *>::const_iterator si = lights.begin();
       si != lights.end(); ++si)
    (*si)->transform (xform);
}


// Add this (or some other) surfaces to the space being built by
// SPACE_BUILDER.
//
void
SurfaceGroup::add_to_space (SpaceBuilder &space_builder) const
{
  for (std::vector<Surface *>::const_iterator si = surfaces.begin();
       si != surfaces.end(); ++si)
    (*si)->add_to_space (space_builder);
}


// Add light-samplers for this surface in SCENE to SAMPLERS.  Any
// samplers added become owned by the owner of SAMPLERS, and will be
// destroyed when it is.
//
void
SurfaceGroup::add_light_samplers (const Scene &scene,
				  std::vector<const Light::Sampler *> &samplers)
  const
{
  for (std::vector<Surface *>::const_iterator si = surfaces.begin();
       si != surfaces.end(); ++si)
    (*si)->add_light_samplers (scene, samplers);

  for (std::vector<Light *>::const_iterator si = lights.begin();
       si != lights.end(); ++si)
    (*si)->add_light_samplers (scene, samplers);
}


// Add statistics about this surface to STATS (see the definition of
// Surface::Stats below for details).  CACHE is used internally for
// coordination amongst nested surfaces.
//
// This method is intended for internal use in the Surface class
// hierachy, but cannot be protected: due to pecularities in the way
// that is defined in C++.
//
void
SurfaceGroup::accum_stats (Stats &stats, StatsCache &cache) const
{
  for (std::vector<Surface *>::const_iterator si = surfaces.begin();
       si != surfaces.end(); ++si)
    (*si)->accum_stats (stats, cache);

  stats.num_lights += lights.size ();
}
