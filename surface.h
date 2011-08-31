// surface.h -- Physical surface
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011  Miles Bader <miles@gnu.org>
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

#include <vector>

#include "ray.h"
#include "bbox.h"
#include "intersect.h"


namespace snogray {

class SpaceBuilder;
class Media;
class Medium;
class Light;


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

  // If this surface, or some part of it, uses any light-emitting
  // materials, add appropriate Light objects to LIGHTS.  Any lights
  // added become owned by the owner of LIGHTS, and will be destroyed
  // when it is.
  //
  virtual void add_lights (std::vector<Light *> & /*lights*/) const { }

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

  // Return the texture-coordinates of this intersection.
  //
  virtual TexCoords tex_coords () const = 0;

  // Return the normal of this intersection (in the world frame).
  //
  virtual Vec normal () const = 0;

  Ray ray;
};



// Surface::Sampler

// An interface for sampling a Surface, which is used for area
// lighting.
//
class Surface::Sampler
{
public:

  // A sample of the surface area.
  //
  struct AreaSample
  {
    AreaSample (const Pos &_pos, const Vec &_norm, float _pdf)
      : pos (_pos), normal (_norm), pdf (_pdf)
    { }
    AreaSample () : pdf (0) { }

    // The point on the surface.
    //
    Pos pos;

    // The surface normal at POS.
    //
    Vec normal;

    // The value of the "probability density function" for this
    // sample, base on the sampling area of (one side of) the
    // surface.
    //
    float pdf;
  };

  // A sample of the surface area from a particular viewpoint.
  //
  // This may or may not be restricted to parts of the surface which
  // are visible from that viewpoint.
  //
  struct AngularSample
  {
    AngularSample (const Vec &_dir, const Vec &_norm,
		   float _pdf, dist_t _dist)
      : dir (_dir), normal (_norm), pdf (_pdf), dist (_dist)
    { }
    AngularSample () : pdf (0) { }

    // This constructor can be used to convert from an area-based
    // sample to an angular sample from a specific viewpoint.
    //
    AngularSample (const AreaSample &area_sample, const Pos &viewpoint);

    // The direction of the sample on the surface from the viewpoint.
    //
    Vec dir;

    // The surface normal at POS.
    //
    Vec normal;

    // The value of the "probability density function" for this
    // sample, based on a hemisphere distribution around the
    // viewpoint.
    //
    float pdf;

    // The distance from the viewpoint to the sample.
    //
    dist_t dist;
  };

  // Return a sample of this surface.
  //
  virtual AreaSample sample (const UV &param) const = 0;

  // Return a sample of this surface from VIEWPOINT, based on the
  // parameter PARAM.
  //
  // This method is optional; the default implementation calls the
  // Surface::Sampler::sample method, and converts the result to an
  // AngularSample.
  //
  virtual AngularSample sample_from_viewpoint (const Pos &viewpoint,
					       const UV &param)
    const;

  // If a ray from VIEWPOINT in direction DIR intersects this
  // surface, return an AngularSample as if the
  // Surface::Sampler::sample_from_viewpoint method had returned a
  // sample at the intersection position.  Otherwise, return an
  // AngularSample with a PDF of zero.
  //
  virtual AngularSample eval_from_viewpoint (const Pos &viewpoint,
					     const Vec &dir)
    const = 0;

protected:

  // This is a helper function that can be used to return a sample
  // with an automatically-calculated, but somewhat approximate,
  // PDF.  The caller passes in a position-sampling functor,
  // POS_SAMPLE_FUN, which will be used to calculate the sample
  // position, the sample parameter PARAM, and a normal NORM.
  //
  // The PDF is calculated by slightly perturbing PARAM in both the
  // U and V directions by a small factor DELTA, and calling
  // POS_SAMPLE_FUN to generate corresponding sample positions,
  // POS_DU and POS_DV.  The PDF is then the ratio of these two
  // "patches" -- one in parameter space (with area DELTA*DELTA) and
  // one in sample space (area |(POS_DU - POS) x (POS_DV - POS)|).
  //
  // The resulting PDF is slightly inaccurate in most cases because
  // of the assumption that the "sample patch" is a parallelogram
  // (it's not really), but if DELTA is reasonably small, it's a
  // pretty good approximation.
  //
  // This is useful in cases where the PDF is hard to calculate
  // analytically (such as surfaces that have an arbitrary transform
  // matrix applied to them).
  //
  template<typename PosSampleFun>
  static Surface::Sampler::AreaSample
  sample_with_approx_area_pdf (const PosSampleFun &pos_sample_fun,
			       const UV &param, const Vec &norm)
  {
    float delta = 0.0001f;  // this value seems to work well
    Pos pos = pos_sample_fun (param);
    Pos pos_du = pos_sample_fun (param + UV (delta, 0));
    Pos pos_dv = pos_sample_fun (param + UV (0, delta));
    float sample_area = cross (pos_du - pos, pos_dv - pos).length ();
    float param_area = delta * delta;
    float pdf = sample_area == 0 ? 0 : param_area / sample_area;
    return AreaSample (pos, norm, pdf);
  }
};


}

#endif // __SURFACE_H__


// arch-tag: 85997b65-c9ab-4542-80be-0c3a114593ba
