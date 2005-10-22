// light.h -- Light object
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "pos.h"

namespace Snogray {

class Intersect;
class Color;
class LightModel;
class TraceState;
class Ray;

class Light
{
public:

  Light (const Pos &_pos) : pos (_pos), num (0) { }
  virtual ~Light (); // stop gcc bitching

  // Return the color of the surface at ISEC, with nominal color
  // SURFACE_COLOR, as lit by this light with lighting-model
  // LIGHT_MODEL.
  //
  virtual Color illum (const Intersect &isec, const Color &surface_color,
		       const LightModel &light_model, TraceState &tstate)
    const = 0;

  // Adjust this light's intensity by a factor of SCALE.
  //
  virtual void scale_intensity (float scale) = 0;

  // Calculate the illumination by a single light ray LIGHT_RAY with
  // color LIGHT_COLOR, adjusting for any shdowing etc. that may  affect
  // it.  LIGHT_RAY points from the surface to the light.
  //
  Color ray_illum (const Ray &light_ray, const Color &light_color,
		   const Intersect &isec, const Color &surface_color,
		   const LightModel &light_model, TraceState &tstate)
    const;

  Pos pos;

  // Each light has a number, which we use as a index to access various
  // data structures referring to lights.
  //
  unsigned num;
};

}

#endif /* __LIGHT_H__ */

// arch-tag: 07d0a36e-d44f-44f8-bb69-e57c9681de14
