// point-light.h -- Point light
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
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

  // Return the color of the surface at ISEC, with nominal color
  // SURFACE_COLOR, as lit by this light with lighting-model
  // LIGHT_MODEL.
  //
  virtual Color illum (const Intersect &isec, const Color &surface_color,
		       const LightModel &light_model, TraceState &tstate)
    const;

  // Adjust this light's intensity by a factor of SCALE.
  //
  virtual void scale_intensity (float scale);

  Pos pos;
  Color color;
};

}

#endif /* __POINT_LIGHT_H__ */

// arch-tag: de5d75ba-f8bd-4dde-a729-37ea8f37a7cc
