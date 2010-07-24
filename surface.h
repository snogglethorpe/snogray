// surface.h -- Physical surface
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

#ifndef __SURFACE_H__
#define __SURFACE_H__

#include "vec.h"
#include "ray.h"
#include "bbox.h"
#include "intersect.h"
#include "material.h"


namespace snogray {

class SpaceBuilder;
class ShadowRay;
class Media;


// A surface is the basic object scenes are constructed of.
// Surfaces exist in 3D space, but are basically 2D -- volumetric
// properties are only modelled in certain special cases.
//
class Surface 
{
public:

  Surface (const Ref<const Material> &mat) : material (mat) { }
  virtual ~Surface () { }

  // A lightweight object used to return infomation from the
  // Surface::intersect method.  If that intersection ends up being used
  // for rendering, its IsecInfo::make_intersect method will be called
  // to create a (more heavyweight) Intersect object for doing
  // rendering.
  //
  // These objects should be allocated using placement new with the
  // RenderContext object passed to Surface::intersect.
  //
  class IsecInfo
  {
  public:

    // XXX get rid RAY argument; see comment below for IsecInfo::ray field
    //
    IsecInfo (const Ray &_ray) : ray (_ray) { }
    virtual ~IsecInfo () { }

    // Create an Intersect object for this intersection.
    //
    virtual Intersect make_intersect (const Media &media, RenderContext &context)
      const = 0;

    // XXX get rid of this field; possible as soon as we remove the old
    // rendering infrastructure.
    //
    Ray ray;
  };

  // If this surface intersects RAY, change RAY's maximum bound (Ray::t1)
  // to reflect the point of intersection, and return a Surface::IsecInfo
  // object describing the intersection (which should be allocated using
  // placement-new with CONTEXT); otherwise return zero.
  //
  virtual const IsecInfo *intersect (Ray &ray, RenderContext &context) const;

  // Return true if this surface intersects RAY.
  //
  virtual bool intersects (const ShadowRay &ray, RenderContext &context) const;

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const;

  // Add this (or some other) surfaces to the space being built by
  // SPACE_BUILDER.
  //
  virtual void add_to_space (SpaceBuilder &space_builder) const;
 
  // The "smoothing group" this surface belongs to, or zero if it belongs
  // to none.  The smoothing group affects shadow-casting: if two objects
  // are in the same smoothing group, they will not be shadowed by
  // back-surface shadows from each other; typically all triangles in a
  // mesh are in the same smoothing group.
  //
  virtual const void *smoothing_group () const;

  Ref<const Material> material;
};


}


#endif /* __SURFACE_H__ */


// arch-tag: 85997b65-c9ab-4542-80be-0c3a114593ba
