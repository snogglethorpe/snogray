// surface-group.h -- Group of surfaces
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

#ifndef SNOGRAY_SURFACE_GROUP_H
#define SNOGRAY_SURFACE_GROUP_H


#include <vector>

#include "surface.h"


namespace snogray {


// A group of surfaces represented as a single composite surface.
//
class SurfaceGroup : public Surface
{
public:

  SurfaceGroup () { }
  ~SurfaceGroup ();

  // Add SURFACE to this group.
  //
  void add (Surface *surface);

  // Add LIGHT to this group.
  //
  void add (Light *light);

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const { return _bbox; }

  // Transform the geometry of this surface by XFORM.
  //
  virtual void transform (const Xform &xform);

  // Add Surface::Renderable objects associated with this surface to
  // the space being built by SPACE_BUILDER.
  //
  virtual void add_to_space (SpaceBuilder &space_builder) const;

  // Add light-samplers for this surface in SCENE to SAMPLERS.  Any
  // samplers added become owned by the owner of SAMPLERS, and will be
  // destroyed when it is.
  //
  virtual void add_light_samplers (
		 const Scene &scene,
		 std::vector<const Light::Sampler *> &samplers)
    const;

  // Add statistics about this surface to STATS (see the definition of
  // Surface::Stats below for details).  CACHE is used internally for
  // coordination amongst nested surfaces.
  //
  // This method is intended for internal use in the Surface class
  // hierachy, but cannot be protected: due to pecularities in the way
  // that is defined in C++.
  //
  virtual void accum_stats (Stats &stats, StatsCache &cache) const;


private:

  // A list of the surfaces in this group.
  //
  std::vector<Surface *> surfaces;

  // A list of explicit lights in this group.
  //
  std::vector<Light *> lights;

  // Cached bounding box for the entire group.
  //
  BBox _bbox;
};


}


#endif /* SNOGRAY_SURFACE_GROUP_H */
