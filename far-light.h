// far-light.h -- Light at infinite distance
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __FAR_LIGHT_H__
#define __FAR_LIGHT_H__

#include "snogmath.h"
#include "light.h"
#include "color.h"
#include "pos.h"

namespace snogray {

// A light at an "infinite" distance.
//
class FarLight : public Light
{
public:

  // ANGLE is the apparent (linear) angle subtended by of the light.
  // INTENSITY is the amount of light emitted per steradian.
  //
  FarLight (const Vec &_dir, float _angle, const Color &_intensity)
    : intensity (_intensity), angle (_angle), frame (_dir.unit ()),
      pdf (1 / angle), min_cos (cos (_angle / 2))
  { }

  // Generate around NUM samples of this light and add them to SAMPLES.
  // Return the actual number of samples (NUM is only a suggestion).
  //
  virtual unsigned gen_samples (const Intersect &isec, unsigned num,
				IllumSampleVec &samples)
    const;

  // For every sample from BEG_SAMPLE to END_SAMPLE which intersects this
  // light, and where light is closer than the sample's previously recorded
  // light distance (or the previous distance is zero), overwrite the
  // sample's light-related fields with information from this light.
  //
  virtual void filter_samples (const Intersect &isec, 
			       const IllumSampleVec::iterator &beg_sample,
			       const IllumSampleVec::iterator &end_sample)
    const;

  // Return a sample of this light from the viewpoint of ISEC (using a
  // surface-normal coordinate system, where the surface normal is
  // (0,0,1)), based on the parameter PARAM.
  //
  virtual Sample sample (const Intersect &isec, const UV &param) const;

  // Evaluate this light in direction DIR from the viewpoint of ISEC (using
  // a surface-normal coordinate system, where the surface normal is
  // (0,0,1)).
  //
  virtual Value eval (const Intersect &isec, const Vec &dir) const;

  Color intensity;
  dist_t angle;

private:
  
  // Frame of reference pointing at this light from the origin.
  //
  const Frame frame;

  // As our light subtends a constant angle, and we sample it uniformly
  // by solid angle, we have a constant pdf.
  //
  const float pdf;

  // The minimum cosine of the angle between a sample and this light.
  // Any samples where the cosine is less than this (meaning the angle
  // between the sample and the light direction is greater) do not hit
  // the light.
  //
  dist_t min_cos;
};

}

#endif /* __FAR_LIGHT_H__ */

// arch-tag: 0691dd09-998d-4cdf-b5e9-da71aed2ec41
