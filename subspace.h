// subspace.h -- A surface encapsulated into its own subspace
//
//  Copyright (C) 2007, 2008, 2009, 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SUBSPACE_H__
#define __SUBSPACE_H__

#include "ref.h"
#include "space.h"
#include "surface.h"
#include "material.h"
#include "unique-ptr.h"
#include "ray.h"
#include "mutex.h"


namespace snogray {


// A surface with its own unique "subspace" (acceleration structure).
//
// This is for use with an Instance.
//
class Subspace : public RefCounted
{
public:

  Subspace (Surface *surf) : surface (surf), space (0) { }
  ~Subspace ();

  // If the associated surface intersects RAY, change RAY's maximum bound
  // (Ray::t1) to reflect the point of intersection, and return a
  // Surface::IsecInfo object describing the intersection (which should be
  // allocated using placement-new with CONTEXT); otherwise return zero.
  //
  const Surface::IsecInfo *intersect (Ray &ray, RenderContext &context) const
  {
    ensure_space (context);
    return space->intersect (ray, context);
  }

  // Return true if something in this subspace intersects RAY.
  //
  bool intersects (const Ray &ray, RenderContext &context) const
  {
    ensure_space (context);
    return space->intersects (ray, context);
  }

  // Return true if some surface in this subspace completely occludes
  // RAY.  If no surface completely occludes RAY, then return false,
  // and multiply TOTAL_TRANSMITTANCE by the transmittance of any
  // surfaces in this subspace which partially occlude RAY, evaluated
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
    ensure_space (context);
    return space->occludes (ray, medium, total_transmittance, context);
  }

  // Return a bounding box for the associated surface.
  //
  BBox bbox () const { return surface->bbox (); }

private:

  // Make sure our acceleration structure is set up.
  //
  void ensure_space (RenderContext &context) const
  {
    if (! space)
      make_space (context);
  }

  // Setup our acceleration structure.
  //
  void make_space (RenderContext &context) const;

  // The top-level surface in this subspace.
  //
  UniquePtr<Surface> surface;

  // Space holding everything from SURFACE..
  //
  mutable const Space *space;

  // A lock used to serialize initialization of the Subspace::space field.
  //
  // Only used by Subspace::make_space (which is only called if
  // Subspace::space is zero).
  //
  mutable Mutex make_space_lock;
};


}


#endif /* __SUBSPACE_H__ */
