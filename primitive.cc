// surface.cc -- Primitive surface
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "surface-light.h"

#include "primitive.h"


using namespace snogray;


// Add a single area light, using this primitive's shape, to LIGHTS,
// with with intensity INTENSITY.  An error will be signaled if this
// primitive does not support lighting.
//
void
Primitive::add_light (const TexVal<Color> &intens, std::vector<Light *> &lights)
  const
{
  lights.push_back (new SurfaceLight (*this, intens));
}
