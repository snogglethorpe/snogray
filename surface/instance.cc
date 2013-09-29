// instance.cc -- Transformed object model
//
//  Copyright (C) 2005-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "intersect/intersect.h"
#include "space/space.h"
#include "util/excepts.h"
#include "model.h"

#include "instance.h"


using namespace snogray;



// Instance::IsecInfo

class Instance::IsecInfo : public Surface::IsecInfo
{
public:

  IsecInfo (const Ray &ray,
	    const Instance &_instance,
	    const Surface::IsecInfo *_model_isec_info)
    : Surface::IsecInfo (ray),
      instance (_instance),
      model_isec_info (_model_isec_info)
  { }

  virtual Intersect make_intersect (const Media &media,
				    RenderContext &context)
    const;

  virtual Vec normal () const;

private:

  const Instance &instance;

  const Surface::IsecInfo *model_isec_info;
};

// Create an Intersect object for this intersection.
//
Intersect
Instance::IsecInfo::make_intersect (const Media &media, RenderContext &context)
  const
{
  // First make an intersection in our model.
  //
  Intersect isec = model_isec_info->make_intersect (media, context);

  // Now transform parts of it to be in the global space.
  //
  isec.normal_frame.origin
    = instance.local_to_world (isec.normal_frame.origin);
  isec.normal_frame.x
    = instance.local_to_world (isec.normal_frame.x).unit ();
  isec.normal_frame.y
    = instance.local_to_world (isec.normal_frame.y).unit ();
  isec.normal_frame.z
    = instance.normal_to_world (isec.normal_frame.z).unit ();

  return isec;
}

// Return the normal of this intersection (in the world frame).
//
Vec
Instance::IsecInfo::normal () const
{
  throw std::runtime_error ("Instance::IsecInfo::normal");
}



// intersection

// If this surface intersects RAY, change RAY's maximum bound (Ray::t1) to
// reflect the point of intersection, and return a Surface::IsecInfo object
// describing the intersection (which should be allocated using
// placement-new with CONTEXT); otherwise return zero.
//
Surface::IsecInfo *
Instance::intersect (Ray &ray, RenderContext &context) const
{
  // Transform the ray for searching our model.
  //
  Ray xformed_ray = world_to_local (ray);

  const Surface::IsecInfo *model_isec_info
    = model->intersect (xformed_ray, context);

  if (model_isec_info)
    {
      ray.t1 = xformed_ray.t1;
      return new (context) IsecInfo (ray, *this, model_isec_info);
    }
  else
    return 0;
}

// Return true if this surface intersects RAY.
//
bool
Instance::intersects (const Ray &ray, RenderContext &context) const
{
  // Transform the ray for searching our model.
  //
  Ray xformed_ray = world_to_local (ray);
  return model->intersects (xformed_ray, context);
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
Instance::occludes (const Ray &ray, const Medium &medium,
		    Color &total_transmittance,
		    RenderContext &context)
  const
{
  // Transform the ray for searching our model.
  //
  Ray xformed_ray = world_to_local (ray);
  return model->occludes (xformed_ray, medium, total_transmittance, context);
}



// misc Instance methods

// Return a bounding box for this surface.
//
BBox
Instance::bbox () const
{
  return local_to_world (model->surface ()->bbox ());
}
