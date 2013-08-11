// point-light.h -- Point light
//
//  Copyright (C) 2005-2007, 2010-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_POINT_LIGHT_H
#define SNOGRAY_POINT_LIGHT_H

#include "util/snogmath.h"
#include "color/color.h"
#include "geometry/frame.h"

#include "light.h"


namespace snogray {


class PointLight : public Light
{
public:

  // A simple point-light that radiates in all directions from POS.
  //
  PointLight (const Pos &pos, const Color &col)
    : frame (pos), color (col), cos_half_angle (-1), cos_half_core_angle (-1)
  { }

  // A point-light that radiates from POS in a cone with an apex angle
  // of ANGLE in direction DIR (in other words, a "spotlight").
  // FRINGE_ANGLE, if supplied, is the angle of a region inside the
  // outer edge where the light falls off towards the edge (otherwise
  // the light inside the cone is a constant intensity).
  //
  PointLight (const Pos &pos, const Color &col, float angle, const Vec &dir,
	      float fringe_angle = 0)
    : frame (pos, dir), color (col),
      cos_half_angle (cos (angle / 2)),
      cos_half_core_angle (cos ((angle - fringe_angle) / 2))
  { }

  // Return a sample of this light from the viewpoint of ISEC (using a
  // surface-normal coordinate system, where the surface normal is
  // (0,0,1)), based on the parameter PARAM.
  //
  virtual Sample sample (const Intersect &isec, const UV &param) const;

  // Return a "free sample" of this light.
  //
  virtual FreeSample sample (const UV &param, const UV &dir_param) const;

  // Evaluate this light in direction DIR from the viewpoint of ISEC (using
  // a surface-normal coordinate system, where the surface normal is
  // (0,0,1)).
  //
  virtual Value eval (const Intersect &isec, const Vec &dir) const;

  // Return true if this is a point light.
  //
  virtual bool is_point_light () const { return true; }

private:

  Color intensity (float cos_dir) const
  {
    Color intens = color;

    // If outside the "core" and inside the "fringe", then ramp down
    // the intensity proportional to the distance towards the final
    // edge.
    //
    if (cos_dir < cos_half_core_angle)
      {
	float d
	  = (cos_dir - cos_half_angle) / (cos_half_core_angle - cos_half_angle);
	intens *= d * d * d * d;
      }

    return intens;
  }

  // A frame with its origin at the light's position, and its z-axis
  // pointing in the direction the light radiates in.  Note that the
  // case ANGLE is 2*PI, the direction is irrelevant.
  //
  Frame frame;

  // Amount and color of light emitted.
  //
  Color color;

  // The cosine of half the angle the light-emitting cone subtends.
  // If -1 (the cosine of PI), then this light radiates in all
  // directions.
  //
  float cos_half_angle;

  // The cosine of half the angle the "core" light-emitting cone
  // subtends; the "core" is that part of the light's beam which is
  // full-intensity.
  //
  float cos_half_core_angle;
};


}

#endif // SNOGRAY_POINT_LIGHT_H


// arch-tag: de5d75ba-f8bd-4dde-a729-37ea8f37a7cc
