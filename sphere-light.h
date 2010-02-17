// sphere-light.h -- Spherical light
//
//  Copyright (C) 2006, 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SPHERE_LIGHT_H__
#define __SPHERE_LIGHT_H__

#include "light.h"
#include "color.h"
#include "pos.h"


namespace snogray {


class SphereLight : public Light
{
public:

  SphereLight (const Pos &_pos, float _radius, const Color &_intensity)
    : pos (_pos), radius (_radius), intensity (_intensity) 
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
  dist_t radius;

  // Radiant emittance of this light (W / m^2).
  //
  Color intensity;

private:

  // Return the solid angle subtended by this light, where LIGHT_CENTER_VEC
  // is a vector from the viewer to the light's center.
  //
  float solid_angle (const Vec &light_center_vec) const;
};


}

#endif // __SPHERE_LIGHT_H__

// arch-tag: e40bcb89-44fb-478a-b8b6-c5265c4537d2
