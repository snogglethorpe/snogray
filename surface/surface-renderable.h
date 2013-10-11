// surface-renderable.h -- Renderable component of surface
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

#ifndef SNOGRAY_SURFACE_RENDERABLE_H
#define SNOGRAY_SURFACE_RENDERABLE_H

#include "geometry/ray.h"
#include "render/intersect.h"

#include "surface.h"


namespace snogray {

class Media;
class Medium;
class RenderContext;


// A Renderable is the interface to a Surface which is used for
// rendering.
//
// This is an abstract class.
//
class Surface::Renderable
{
public:

  class IsecInfo;	  // Used to return info about an intersection

  virtual ~Renderable () {}

  // If this surface intersects RAY, change RAY's maximum bound
  // (Ray::t1) to reflect the point of intersection, and return a
  // Surface::Renderable::IsecInfo object describing the intersection
  // (which should be allocated using placement-new with CONTEXT);
  // otherwise return zero.
  //
  virtual const IsecInfo *intersect (Ray &ray, RenderContext &context) const =0;

  // Return true if this surface intersects RAY.
  //
  virtual bool intersects (const Ray &ray, RenderContext &context) const = 0;

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
  // [This interface is slight awkward for reasons of speed --
  // returning and checking for a boolean value for common cases is
  // significantly faster than, for instance, a simple "transmittance"
  // method, which requires handling Color values for all cases.]
  //
  virtual bool occludes (const Ray &ray, const Medium &medium,
			 Color &total_transmittance,
			 RenderContext &context)
    const = 0;

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const = 0;
};



// ----------------------------------------------------------------
// Surface::Renderable::IsecInfo


// A lightweight object used to return infomation from the
// Surface::Renderable::intersect method.  If that intersection ends
// up being used for rendering, its IsecInfo::make_intersect method
// will be called to create a (more heavyweight) Intersect object for
// doing rendering.
//
// These objects should be allocated using placement new with the
// RenderContext object passed to Surface::Renderable::intersect.
// Their destructor is never called (and so should all be trivial and
// non-virtual).
//
class Surface::Renderable::IsecInfo
{
public:

  IsecInfo (const Ray &_ray) : ray (_ray) { }

  // Note: Surface::Renderable::IsecInfo does not have a virtual
  // destructor (and subclasses should not add one)!
  //
  // Indeed, IsecInfo destructors should never be called at all --
  // allocation of IsecInfo objects is done using a special arena, and
  // they are never explicitly destroyed.  Therefore, all subclasses
  // of Surface::Renderable::IsecInfo should have trivial destructors,
  // and should never contain any information which needs to be
  // destroyed.

  // Create an Intersect object for this intersection.
  //
  virtual Intersect make_intersect (const Media &media, RenderContext &context)
    const = 0;

  // Return the normal of this intersection (in the world frame).
  //
  virtual Vec normal () const = 0;

  Ray ray;
};


}

#endif // SNOGRAY_SURFACE_RENDERABLE_H
