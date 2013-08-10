// surface-light.h -- General-purpose area light
//
//  Copyright (C) 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_SURFACE_LIGHT_H
#define SNOGRAY_SURFACE_LIGHT_H

#include "unique-ptr.h"
#include "color.h"
#include "texture/tex.h"
#include "light.h"
#include "surface.h"


namespace snogray {


class SurfaceLight : public Light
{
public:

  SurfaceLight (const Surface &surface, const TexVal<Color> &_intensity);

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

  // A sampler for the surface which is lit.
  //
  UniquePtr<const Surface::Sampler> sampler;

private:

  // Radiant emittance of this light (W / m^2).
  //
  Color intensity;
};


}

#endif // SNOGRAY_SURFACE_LIGHT_H
