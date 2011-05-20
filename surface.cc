// surface.cc -- Physical surface
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

#include <stdexcept>

#include "surface.h"
#include "space.h"

using namespace snogray;


// Add this (or some other) surfaces to the space being built by
// SPACE_BUILDER.
//
void
Surface::add_to_space (SpaceBuilder &space_builder) const
{
  space_builder.add (this);
}

// Stubs -- these should be abstract methods, but C++ doesn't allow a
// class with abstract methods to be used in a list/vector, so we just
// signal a runtime error if they're ever called.

static void barf () __attribute__ ((noreturn));
static void
barf ()
{
  throw std::runtime_error ("tried to render abstract surface");
}

const Surface::IsecInfo *
Surface::intersect (Ray &, RenderContext &) const { barf (); }
bool Surface::intersects (const Ray &, RenderContext &) const { barf (); }
bool Surface::occludes (const Ray &, const Medium &, Color &, RenderContext &) const { barf ();}
BBox Surface::bbox () const { barf (); }



// Surface::Sampler

// This constructor can be used to convert from an area-based
// sample to an angular sample from a specific viewpoint.
//
// If AREA_SAMPLE's normal points away from VIEWPOINT, meaning only the
// back-surface of the surface is visible from VIEWPOINT at that
// point, then the PDF of the angular sample will be set to zero.
//
Surface::Sampler::AngularSample::AngularSample (const AreaSample &area_sample,
						const Pos &viewpoint)
  : normal (area_sample.normal), pdf (0)
{
  Vec view_vec = area_sample.pos - viewpoint;

  // Distance to the sample.
  //
  dist = view_vec.length ();

  if (dist > 0)
    {
      dist_t inv_dist = 1 / dist;

      // DIR is a unit vector pointing towards the sample in normal-space.
      //
      dir = view_vec * inv_dist;

      // PDF_COS_ADJ is a "cosine factor":
      //
      //    cos (angle_between (-sample_normal, sample_dir))
      //  = dot (-sample_normal, sample_dir)
      //
      // It adjusts for the surface normal not being parallel to DIR.
      // As the normal rotates away from DIR, the samples get
      // scrunched up, so the PDF goes up.
      //
      float pdf_cos_adj = dot (-normal, dir);

      // If PDF_COS_ADJ is negative then the light points away from
      // ISEC, and so can have no effect.
      //
      if (pdf_cos_adj > 0)
	{
	  // Area to solid-angle conversion, dw/dA, where w is a solid
	  // angle in the hemisphere visible from the origin of VIEW_VEC.
	  //
	  float dw_dA = pdf_cos_adj * inv_dist * inv_dist;
	  pdf = area_sample.pdf / dw_dA;
	}
    }
}

// Return a sample of this surface from VIEWPOINT, based on the
// parameter PARAM.
//
// This method is optional; the default implementation calls the
// Surface::Sampler::sample method, and converts the result to an
// AngularSample.
//
Surface::Sampler::AngularSample
Surface::Sampler::sample_from_viewpoint (const Pos &viewpoint, const UV &param)
  const
{
  return AngularSample (sample (param), viewpoint);
}


// arch-tag: a62e1854-d7ca-4cb3-a8dc-9be328c53430
