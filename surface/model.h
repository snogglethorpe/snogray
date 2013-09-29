// model.h -- A surface encapsulated into its own model
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

#ifndef SNOGRAY_MODEL_H
#define SNOGRAY_MODEL_H

#include "util/ref.h"
#include "space/space.h"
#include "space/space-builder.h"
#include "surface.h"
#include "util/unique-ptr.h"
#include "intersect/ray.h"
#include "util/mutex.h"


namespace snogray {


class SpaceBuilderFactory;


// A "model" surface, which can be replicated multiple times using
// Instances (a model cannot be rendered directly, only via an
// Instance).
//
class Model : public RefCounted
{
public:

  Model (Surface *surf, const SpaceBuilderFactory &space_builder_factory);

  // If the associated surface intersects RAY, change RAY's maximum bound
  // (Ray::t1) to reflect the point of intersection, and return a
  // Surface::IsecInfo object describing the intersection (which should be
  // allocated using placement-new with CONTEXT); otherwise return zero.
  //
  const Surface::IsecInfo *intersect (Ray &ray, RenderContext &context) const
  {
    ensure_space ();
    return space->intersect (ray, context);
  }

  // Return true if something in this model intersects RAY.
  //
  bool intersects (const Ray &ray, RenderContext &context) const
  {
    ensure_space ();
    return space->intersects (ray, context);
  }

  // Return true if some surface in this model completely occludes
  // RAY.  If no surface completely occludes RAY, then return false,
  // and multiply TOTAL_TRANSMITTANCE by the transmittance of any
  // surfaces in this model which partially occlude RAY, evaluated
  // in medium MEDIUM.
  //
  // Note that this method does not try to handle non-trivial forms of
  // transparency/translucency (for instance, a "glass" material is
  // probably considered opaque because it changes light direction as
  // well as transmitting it), nor does it deal with anything except
  // surfaces.
  //
  virtual bool occludes (const Ray &ray, const Medium &medium,
			 Color &total_transmittance,
			 RenderContext &context)
    const
  {
    ensure_space ();
    return space->occludes (ray, medium, total_transmittance, context);
  }

  // Return a pointer to the model's actual surface.  The returned
  // pointer is only valid while the model still exists.
  //
  Surface *surface () const { return _surface.get (); }

private:

  // Make sure our acceleration structure is set up.
  //
  void ensure_space () const { if (! space) make_space (); }

  // Setup our acceleration structure.
  //
  void make_space () const;

  // The top-level surface in this model.
  //
  UniquePtr<Surface> _surface;

  // Space holding everything from SURFACE, or zero if it hasn't been
  // built yet.
  //
  mutable UniquePtr<const Space> space;

  // SpaceBuilder that can be used to build SPACE, or zero if it's
  // already been built.
  //
  mutable UniquePtr<SpaceBuilder> space_builder;

  // A lock used to serialize initialization of the Model::space field.
  //
  // Only used by Model::make_space (which is only called if
  // Model::space is zero).
  //
  mutable Mutex make_space_lock;
};


}

#endif // SNOGRAY_MODEL_H
