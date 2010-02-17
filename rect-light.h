// rect-light.h -- Rectangular light
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
      frame (_pos, cross (_side2, _side1).unit ()) // XXX align w/ side1&side2
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

  // Location and size of the light.
  //
  Pos pos;
  Vec side1, side2;

  // Radiant emittance of this light (W / m^2).
  //
  Color intensity;

  float area;

  // A frame of reference for the light.  This is somewhat redundant,
  // but convenient.
  //
  Frame frame;
};


}

#endif // __RECT_LIGHT_H__

// arch-tag: d78e86c5-6efa-42ba-811f-332ea423090c
