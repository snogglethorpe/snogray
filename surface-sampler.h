// surface-sampler.h -- Sampling interface for surfaces
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

#ifndef SNOGRAY_SURFACE_SAMPLER_H
#define SNOGRAY_SURFACE_SAMPLER_H

#include "surface.h"


namespace snogray {


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

#endif // SNOGRAY_SURFACE_SAMPLER_H
