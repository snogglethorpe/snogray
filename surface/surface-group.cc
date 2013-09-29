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
  for (std::vector<const Surface *>::const_iterator si = surfaces.begin();
       si != surfaces.end(); ++si)
    delete *si;

  for (std::vector<const Light *>::const_iterator li = lights.begin();
       li != lights.end(); ++li)
    delete *li;
}



// Add SURFACE to this group.
//
void
SurfaceGroup::add (const Surface *surface)
{
  surfaces.push_back (surface);
  _bbox += surface->bbox ();
}

// Add LIGHT to this group.
//
void
SurfaceGroup::add (const Light *light)
{
  lights.push_back (light);
}


// Add this (or some other) surfaces to the space being built by
// SPACE_BUILDER.
//
void
SurfaceGroup::add_to_space (SpaceBuilder &space_builder) const
{
  for (std::vector<const Surface *>::const_iterator si = surfaces.begin();
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
  for (std::vector<const Surface *>::const_iterator si = surfaces.begin();
       si != surfaces.end(); ++si)
    (*si)->add_light_samplers (scene, samplers);

  for (std::vector<const Light *>::const_iterator si = lights.begin();
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
  for (std::vector<const Surface *>::const_iterator si = surfaces.begin();
       si != surfaces.end(); ++si)
    (*si)->accum_stats (stats, cache);

  stats.num_lights += lights.size ();
}



// SurfaceGroup intersection methods
//
// These are not normally called during rendering -- instead,
// individual component surfaces of a SurfaceGroup are added to
// acceleration structures, and their intersection methods are called
// instead -- but are provided for completeness, and to make the
// SurfaceGroup class concrete (Surface is an abstract class).


// If this surface intersects RAY, change RAY's maximum bound (Ray::t1) to
// reflect the point of intersection, and return a Surface::IsecInfo object
// describing the intersection (which should be allocated using
// placement-new with CONTEXT); otherwise return zero.
//
const Surface::IsecInfo *
SurfaceGroup::intersect (Ray &ray, RenderContext &context) const
{
  const Surface::IsecInfo *closest = 0;

  for (std::vector<const Surface *>::const_iterator si = surfaces.begin();
       si != surfaces.end(); ++si)
    {
      const Surface::IsecInfo *isec_info = (*si)->intersect (ray, context);
      if (isec_info)
	closest = isec_info;
    }

  return closest;
}

// Return true if this surface intersects RAY.
//
bool
SurfaceGroup::intersects (const Ray &ray, RenderContext &context) const
{
  for (std::vector<const Surface *>::const_iterator si = surfaces.begin();
       si != surfaces.end(); ++si)
    if ((*si)->intersects (ray, context))
      return true;

  return false;
}

// Return true if this surface completely occludes RAY.  If it does
// not completely occlude RAY, then return false, and multiply
// TOTAL_TRANSMITTANCE by the transmittance of the surface in medium
// MEDIUM.
//
// Note that this method does not try to handle non-trivial forms of
// transparency/translucency (for instance, a "glass" material is
// probably considered opaque because it changes light direction as
// well as transmitting it).
//
bool
SurfaceGroup::occludes (const Ray &ray, const Medium &medium,
			Color &total_transmittance,
			RenderContext &context)
  const
{
  for (std::vector<const Surface *>::const_iterator si = surfaces.begin();
       si != surfaces.end(); ++si)
    if ((*si)->occludes (ray, medium, total_transmittance, context))
      return true;

  return false;
}
