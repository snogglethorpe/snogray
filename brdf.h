// brdf.h -- Bi-directional reflectance distribution function
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __BRDF_H__
#define __BRDF_H__

#include "illum-sample.h"


namespace snogray {

class Intersect;


// A Brdf represents the state of a Material object at a particular
// intersection, and is used for rendering calculations.
//
// Expensive operations (such as evaluating textures) can be done when
// creating the Brdf.
//
class Brdf
{
public:

  Brdf (const Intersect &_isec) : isec (_isec) { }
  virtual ~Brdf () {}

  // Generate around NUM samples of this BRDF and add them to SAMPLES.
  // Return the actual number of samples (NUM is only a suggestion).
  //
  virtual unsigned gen_samples (unsigned num, IllumSampleVec &samples)
    const = 0;

  // Add reflectance information for this BRDF to samples from BEG_SAMPLE
  // to END_SAMPLE.
  //
  virtual void filter_samples (const IllumSampleVec::iterator &beg_sample,
			       const IllumSampleVec::iterator &end_sample)
    const = 0;

  // The intersection where this Brdf was created.
  //
  const Intersect &isec;
};


}

#endif /* __BRDF_H__ */


// arch-tag: 8360ddd7-dc17-40b8-8319-8f6d61fe62bf
