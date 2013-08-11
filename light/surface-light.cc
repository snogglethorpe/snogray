// surface-light.cc -- General-purpose area light
//
//  Copyright (C) 2010, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <stdexcept>

#include "intersect/intersect.h"
#include "material/cos-dist.h"
#include "scene.h"
#include "surface/surface-sampler.h"

#include "surface-light.h"


using namespace snogray;


SurfaceLight::SurfaceLight (const Surface &surface,
			    const TexVal<Color> &_intensity)
  : sampler (surface.make_sampler ()), intensity (_intensity.default_val)
{
  if (! sampler)
    throw std::runtime_error
      ("Surface cannot be used as a light");

  if (_intensity.tex)
    throw std::runtime_error
      ("textured intensity not supported by SurfaceLight");
}


// SurfaceLight::sample

// Return a sample of this light from the viewpoint of ISEC (using a
// surface-normal coordinate system, where the surface normal is
// (0,0,1)), based on the parameter PARAM.
//
Light::Sample
SurfaceLight::sample (const Intersect &isec, const UV &param) const
{
  // Sample the surface, in world-space.
  //
  Pos ipos = isec.normal_frame.origin;
  Surface::Sampler::AngularSample samp
    = sampler->sample_from_viewpoint (ipos, param);

  if (samp.pdf > 0)
    {
      // Convert the sample direction to ISEC's normal-space.
      //
      Vec dir = isec.normal_frame.to (samp.dir);

      // Only process samples which are in front of ISEC.
      //
      if (dir.z > 0)
	return Sample (intensity, samp.pdf, dir, samp.dist);
    }

  return Sample ();
}


// SurfaceLight::sample, free-sampling variant

// Return a "free sample" of this light.
//
Light::FreeSample
SurfaceLight::sample (const UV &param, const UV &dir_param) const
{
  // Sample the surface, in world-space.
  //
  Surface::Sampler::AreaSample samp = sampler->sample (param);

  // Choose a direction in SAMP's normal-frame-of-reference according
  // to DIR_PARAM.
  //
  CosDist dist;
  Vec norm_dir = dist.sample (dir_param);

  // Convert NORM_DIR to the world frame-of-reference.
  //
  Vec dir = Frame (samp.normal).from (norm_dir);

  // The PDF is actually POS_PDF * (DIR_PDF * (dA/dw)), where DIR_PDF
  // is the distribution DIST's PDF for DIR, in angular terms, and
  // (dA/dw) is a conversion factor from angular to area terms.
  //
  // However, as we know that DIST is a cosine distribution, whose PDF
  // is cos(theta)/pi (where theta is the angle between DIR and the
  // distribution normal), and since (dA/dw) is 1/cos(theta), the
  // cosine terms cancel out, and we can just use POS_PDF / pi
  // instead.
  //
  float pdf = samp.pdf * INV_PIf;

  return FreeSample (intensity, pdf, samp.pos, dir);
}


// SurfaceLight::eval

// Evaluate this light in direction DIR from the viewpoint of ISEC
// (using a surface-normal coordinate system, where the surface normal
// is (0,0,1)).
//
Light::Value
SurfaceLight::eval (const Intersect &isec, const Vec &dir) const
{
  // Test whether RAY hits the surface, and if so, get a sample
  // parameter describing where it hits.
  //
  Pos ipos = isec.normal_frame.origin;
  Vec wdir = isec.normal_frame.from (dir);
  Surface::Sampler::AngularSample samp
    = sampler->eval_from_viewpoint (ipos, wdir);

  if (samp.pdf > 0)
    return Value (intensity, samp.pdf, samp.dist);

  return Value ();
}


// arch-tag: 60165b73-d34e-4f49-9a90-958daefdeb78
