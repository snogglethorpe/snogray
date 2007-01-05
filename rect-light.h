// rect-light.h -- Rectangular light
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __RECT_LIGHT_H__
#define __RECT_LIGHT_H__

#include "light.h"
#include "color.h"
#include "pos.h"

namespace snogray {

class RectLight : public Light
{
public:

  RectLight (const Pos &_pos, const Vec &_side1, const Vec &_side2,
	     const Color &_intensity)
    : pos (_pos), side1 (_side1), side2 (_side2),
      intensity (_intensity), area (cross (side1, side2).length ()),
      normal (cross (_side1, _side2).unit ())
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

  // Location and size of the light.
  //
  Pos pos;
  Vec side1, side2;

  // Radiant emittance of this light (W / m^2).
  //
  Color intensity;

  float area;

private:

  Vec normal;
};

}

#endif /* __RECT_LIGHT_H__ */

// arch-tag: d78e86c5-6efa-42ba-811f-332ea423090c
