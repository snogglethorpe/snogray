// instance.h -- Transformed virtual instance of a surface
//
//  Copyright (C) 2007-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_INSTANCE_H
#define SNOGRAY_INSTANCE_H

#include "geometry/xform.h"
#include "model.h"

#include "local-surface.h"


namespace snogray {


class Instance : public LocalSurface
{
public:

  class Renderable;		// Renderable part of an instance

  Instance (const Ref<Model> &_model, const Xform &local_to_world_xform)
    : LocalSurface (local_to_world_xform), model (_model)
  { }

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const;

  // Add Surface::Renderable objects associated with this surface to
  // the space being built by SPACE_BUILDER.
  //
  virtual void add_to_space (SpaceBuilder &space_builder) const;

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

  // Model that we're transforming.
  //
  Ref<Model> model;
};


}


#endif // SNOGRAY_INSTANCE_H
