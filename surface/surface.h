// surface.h -- Physical surface
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

#ifndef SNOGRAY_SURFACE_H
#define SNOGRAY_SURFACE_H

#include <vector>
#include <map>

#include "light/light.h"
#include "geometry/bbox.h"


namespace snogray {

class SpaceBuilder;


// A surface is the basic object scenes are constructed of.
// Surfaces exist in 3D space, but are basically 2D -- volumetric
// properties are only modelled in certain special cases.
//
// This is an abstract class.
//
class Surface 
{
public:

  class Renderable;	  // Interface for surface rendering
  class Sampler;	  // Surface-sampling interface
  struct Stats;		  // Surface statistics


  Surface () { }
  virtual ~Surface () { }


  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const = 0;

  // Add Surface::Renderable objects associated with this surface to
  // the space being built by SPACE_BUILDER.
  //
  virtual void add_to_space (SpaceBuilder &space_builder) const = 0;

  // Add light-samplers for this surface in SCENE to SAMPLERS.  Any
  // samplers added become owned by the owner of SAMPLERS, and will be
  // destroyed when it is.
  //
  virtual void add_light_samplers (
		 const Scene &/*scene*/,
		 std::vector<const Light::Sampler *> &/*samplers*/)
    const
  { }

  // Return a sampler for this surface, or zero if the surface doesn't
  // support sampling.  The caller is responsible for destroying
  // returned samplers.
  //
  virtual Sampler *make_sampler () const { return 0; }

  // Return statistics about this surface (see the definition of
  // Surface::Stats below for details).
  //
  Stats stats () const;

  // A cache of already-calculated statistics, for use by
  // Surface::accum_stats (this is only used by instances).
  //
  typedef std::map<const Surface *, const Stats> StatsCache;

  // Add statistics about this surface to STATS (see the definition of
  // Surface::Stats below for details).  CACHE is used internally for
  // coordination amongst nested surfaces.
  //
  // This method is intended for internal use in the Surface class
  // hierachy, but cannot be protected: due to pecularities in the way
  // that is defined in C++.
  //
  virtual void accum_stats (Stats &stats, StatsCache &cache) const = 0;
};



// ----------------------------------------------------------------
// Surface::Stats


// Surface::Stats
struct Surface::Stats
{
  Stats () : num_render_surfaces (0), num_real_surfaces (0), num_lights (0) { }

  Stats &operator+= (const Stats &stats);

  // Number of surfaces taking place in rendering, including virtual
  // instances.  This is roughly the rendering complexity of the
  // scene.
  //
  unsigned long num_render_surfaces;

  // Number of surfaces that actually take up space in memory, not
  // including container-only surfaces.  This is roughly the memory
  // complexity of the scene.
  //
  unsigned long num_real_surfaces;

  // Number of lights participating in rendering.  [We don't currently
  // support support instanced lights, but maybe lights will need a
  // similar split to the above?]
  //
  unsigned long num_lights;
};


}

#endif // SNOGRAY_SURFACE_H
