// rect-light.h -- Rectangular light
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
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

namespace Snogray {

class RectLight : public Light
{
public:

  static const unsigned JITTER_STEPS = 5;

  RectLight (const Pos &_pos, const Vec &_side1, const Vec &_side2,
	     const Color &col)
    : pos (_pos), side1 (_side1), side2 (_side2),
      color (col), normal (_side1.cross (_side2).unit ())
  { }

  // Return the color as lit by this light of the surface at ISEC, with
  // nominal color SURFACE_COLOR and reflectance function BRDF.
  //
  virtual Color illum (const Intersect &isec, const Color &surface_color,
		       const Brdf &brdf, TraceState &tstate)
    const;

  // Adjust this light's intensity by a factor of SCALE.
  //
  virtual void scale_intensity (float scale);

  Pos pos;
  Vec side1, side2;

  Color color;

private:

  Vec normal;
};

}

#endif /* __RECT_LIGHT_H__ */

// arch-tag: d78e86c5-6efa-42ba-811f-332ea423090c
