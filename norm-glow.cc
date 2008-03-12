// norm-glow.cc -- Material whose color indicates surface normal
//
//  Copyright (C) 2007, 2008  Miles Bader <miles@gnu.org>
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


// Return emitted radiance from this light, at the point described by ISEC.
//
Color
NormGlow::le (const Intersect &isec) const
{
  const Vec &n = isec.normal_frame.z;
  return Color (n.x * 0.5f + 0.5f, n.y * 0.5f + 0.5f, n.z * 0.5f + 0.5f);
}
