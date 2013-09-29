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
#include "surface-renderable.h"
#include "model.h"

#include "instance.h"


using namespace snogray;



// Instance::Renderable

class Instance::Renderable : public Surface::Renderable
{
public:

  Renderable (const Instance &_instance) : instance (_instance) { }

  // If this surface intersects RAY, change RAY's maximum bound
  // (Ray::t1) to reflect the point of intersection, and return a
  // Surface::Renderable::IsecInfo object describing the intersection
  // (which should be allocated using placement-new with CONTEXT);
  // otherwise return zero.
  //
  virtual IsecInfo *intersect (Ray &ray, RenderContext &context) const;

  // Return true if this surface intersects RAY.
  //
  virtual bool intersects (const Ray &ray, RenderContext &context) const;

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
  virtual bool occludes (const Ray &ray, const Medium &medium,
			 Color &total_transmittance,
			 RenderContext &context)
    const;

  // Return a bounding box for this surface.
  //
  virtual BBox bbox () const { return instance.bbox (); }

private:

  class IsecInfo;

  const Instance &instance;
};



// Instance::Renderable::IsecInfo

class Instance::Renderable::IsecInfo : public Surface::Renderable::IsecInfo
{
public:

  IsecInfo (const Ray &ray,
	    const Instance &_instance,
	    const Surface::Renderable::IsecInfo *_model_isec_info)
    : Surface::Renderable::IsecInfo (ray),
      instance (_instance),
      model_isec_info (_model_isec_info)
  { }

  virtual Intersect make_intersect (const Media &media,
				    RenderContext &context)
    const;

  virtual Vec normal () const;

private:

  const Instance &instance;

  const Surface::Renderable::IsecInfo *model_isec_info;
};

// Create an Intersect object for this intersection.
//
Intersect
Instance::Renderable::IsecInfo::make_intersect (const Media &media,
						RenderContext &context)
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
Instance::Renderable::IsecInfo::normal () const
{
  throw std::runtime_error ("Instance::Renderable::IsecInfo::normal");
}



// intersection

// If this surface intersects RAY, change RAY's maximum bound
// (Ray::t1) to reflect the point of intersection, and return a
// Surface::Renderable::IsecInfo object describing the intersection
// (which should be allocated using placement-new with CONTEXT);
// otherwise return zero.
//
Surface::Renderable::IsecInfo *
Instance::Renderable::intersect (Ray &ray, RenderContext &context) const
{
  // Transform the ray for searching our model.
  //
  Ray xformed_ray = instance.world_to_local (ray);

  const Surface::Renderable::IsecInfo *model_isec_info
    = instance.model->intersect (xformed_ray, context);

  if (model_isec_info)
    {
      ray.t1 = xformed_ray.t1;
      return new (context) IsecInfo (ray, instance, model_isec_info);
    }
  else
    return 0;
}

// Return true if this surface intersects RAY.
//
bool
Instance::Renderable::intersects (const Ray &ray, RenderContext &context) const
{
  // Transform the ray for searching our model.
  //
  Ray xformed_ray = instance.world_to_local (ray);
  return instance.model->intersects (xformed_ray, context);
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
Instance::Renderable::occludes (const Ray &ray, const Medium &medium,
		    Color &total_transmittance,
		    RenderContext &context)
  const
{
  // Transform the ray for searching our model.
  //
  Ray xformed_ray = instance.world_to_local (ray);
  return instance.model->occludes (xformed_ray, medium, total_transmittance, context);
}



// misc Instance methods

// Return a bounding box for this surface.
//
BBox
Instance::bbox () const
{
  return local_to_world (model->surface ()->bbox ());
}

// Add this (or some other) surfaces to the space being built by
// SPACE_BUILDER.
//
void
Instance::add_to_space (SpaceBuilder &space_builder) const
{
  Renderable *renderable = new Renderable (*this);
  space_builder.add (renderable);
  space_builder.delete_after_rendering (renderable);
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
Instance::accum_stats (Stats &stats, StatsCache &cache) const
{
  // We only compute stats for a single model once, and keep a pointer
  // to it in CACHE for subsequent references.

  const Surface *surface = model->surface ();
  StatsCache::iterator cached_stats = cache.find (surface);

  if (cached_stats == cache.end ())
    {
      // First use of this model, compute its stats and add them to STATS.

      Stats model_stats;
      surface->accum_stats (model_stats, cache);

      cache.insert (std::make_pair (surface, model_stats));

      stats += model_stats;
    }
  else
    {
      // Re-use of this model, use already-computed stats and only add
      // some of its fields to STATS.

      const Stats &model_stats = cached_stats->second;
      stats.num_render_surfaces += model_stats.num_render_surfaces;
    }
}
