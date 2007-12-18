// norm-glow.cc -- Material whose color indicates surface normal
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "intersect.h"

#include "norm-glow.h"

using namespace snogray;

Color
NormGlow::render (const Intersect &isec) const
{
  const Vec &n = isec.normal_frame.z;
  return Color (intens * n.x, intens * n.y, intens * n.z);
}
