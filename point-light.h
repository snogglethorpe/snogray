// point-light.h -- Point light
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __POINT_LIGHT_H__
#define __POINT_LIGHT_H__

#include "light.h"
#include "color.h"
#include "pos.h"

namespace Snogray {

class PointLight : public Light
{
public:

  PointLight (const Pos &_pos, const Color &col) : pos (_pos), color (col) { }

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
  virtual void filter_samples (const Intersect &,
			       const IllumSampleVec::iterator &,
			       const IllumSampleVec::iterator &)
    const
  {
    // A point light can never be hit by an externally generated sample.
  }

  // Return true if this is a point light.
  //
  virtual bool is_point_light () const { return true; }

  Pos pos;
  Color color;
};

}

#endif /* __POINT_LIGHT_H__ */

// arch-tag: de5d75ba-f8bd-4dde-a729-37ea8f37a7cc
