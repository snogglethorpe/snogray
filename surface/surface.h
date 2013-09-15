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

#include "light/light.h"
#include "geometry/bbox.h"
#include "intersect/ray.h"
#include "intersect/intersect.h"


namespace snogray {

class SpaceBuilder;
class Media;
class Medium;


// A surface is the basic object scenes are constructed of.
// Surfaces exist in 3D space, but are basically 2D -- volumetric
// properties are only modelled in certain special cases.
//
// This is an abstract class.
//
class Surface 
{
public:

  class IsecInfo;	  // Used to return info about an intersection
  class Sampler;	  // Surface-sampling interface


  Surface () { }
  virtual ~Surface () { }


  // If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
  // to reflect the point of intersection, and return a Surface::IsecInfo
  // object describing the intersection (which should be allocated using
  // placement-new with CONTEXT); otherwise return zero.
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

  // Add this (or some other) surfaces to the space being built by
  // SPACE_BUILDER.
  //
  virtual void add_to_space (SpaceBuilder &space_builder) const;

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
};



// ----------------------------------------------------------------
// Surface::IsecInfo


// A lightweight object used to return infomation from the
// Surface::intersect method.  If that intersection ends up being used
// for rendering, its IsecInfo::make_intersect method will be called
// to create a (more heavyweight) Intersect object for doing
// rendering.
//
// These objects should be allocated using placement new with the
// RenderContext object passed to Surface::intersect.  Their
// destructor is never called (and so should all be trivial and
// non-virtual).
//
class Surface::IsecInfo
{
public:

  IsecInfo (const Ray &_ray) : ray (_ray) { }

  // Note: Surface::IsecInfo does not have a virtual destructor (and
  // subclasses should not add one)!
  //
  // Indeed, IsecInfo destructors should never be called at all --
  // allocation of IsecInfo objects is done using a special arena, and
  // they are never explicitly destroyed.  Therefore, all subclasses
  // of Surface::IsecInfo should have trivial destructors, and should
  // never contain any information which needs to be destroyed.

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

#endif // SNOGRAY_SURFACE_H
