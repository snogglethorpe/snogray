// light.h -- Light object
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

#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "illum-sample.h"


namespace snogray {


class Intersect;

class Light
{
public:

  struct Sample
  {
    Sample (const Color &_val, float _pdf, const Vec &_dir, dist_t _dist)
      : val (_val), pdf (_pdf), dir (_dir), dist (_dist)
    { }
    Sample () : val (0), pdf (0), dist (0) { }

    // The value of the light for this sample.
    //
    Color val;

    // The value of the "probability density function" for this sample in the
    // light's sample distribution.
    //
    // However, if this is a specular (point) light, the value is not
    // defined (theoretically the value is infinity for specular samples).
    //
    float pdf;

    // The sample direction (the origin is implicit), in the
    // surface-normal coordinate system (where the surface normal is
    // (0,0,1)).
    //
    Vec dir;

    // How far the surface of the light is.
    //
    dist_t dist;
  };

  struct Value
  {
    Value (const Color &_val, float _pdf, dist_t _dist)
      : val (_val), pdf (_pdf), dist (_dist)
    { }
    Value () : val (0), pdf (0), dist (0) { }

    // The value of the light for this value.
    //
    Color val;

    // The value of the "probability density function" for this value in the
    // light's value distribution.
    //
    // However, if this is a specular (point) light, the pdf is not defined
    // (theoretically the value is infinity for specular values).
    //
    float pdf;

    // How far the surface of the light is.
    //
    dist_t dist;
  };

  Light () : num (0) { }
  virtual ~Light () { }

  // Generate around NUM samples of this light and add them to SAMPLES.
  // Return the actual number of samples (NUM is only a suggestion).
  //
  virtual unsigned gen_samples (const Intersect &isec, unsigned num,
				IllumSampleVec &samples)
    const = 0;

  // For every sample from BEG_SAMPLE to END_SAMPLE which intersects this
  // light, and where light is closer than the sample's previously recorded
  // light distance (or the previous distance is zero), overwrite the
  // sample's light-related fields with information from this light.
  //
  virtual void filter_samples (const Intersect &isec, 
			       const IllumSampleVec::iterator &beg_sample,
			       const IllumSampleVec::iterator &end_sample)
    const = 0;

  // Return a sample of this light from the viewpoint of ISEC (using a
  // surface-normal coordinate system, where the surface normal is
  // (0,0,1)), based on the parameter PARAM.
  //
  virtual Sample sample (const Intersect &isec, const UV & /*param*/) const = 0;

  // Evaluate this light in direction DIR from the viewpoint of ISEC (using
  // a surface-normal coordinate system, where the surface normal is
  // (0,0,1)).
  //
  virtual Value eval (const Intersect &isec, const Vec &dir) const = 0;

  // Return true if this is a point light.
  //
  virtual bool is_point_light () const { return false; }

  // Each light has a number, which we use as a index to access various
  // data structures referring to lights.
  //
  unsigned num;
};


}

#endif /* __LIGHT_H__ */

// arch-tag: 07d0a36e-d44f-44f8-bb69-e57c9681de14
