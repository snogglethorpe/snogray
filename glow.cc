// glow.cc -- Constant-color reflectance function
//
//  Copyright (C) 2005, 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "glow.h"
#include "scene.h"		// for inlined Trace::shadow method

using namespace snogray;


// Return emitted radiance from this light, at the point described by ISEC.
//
Color
Glow::le (const Intersect &isec) const
{
  return isec.back ? 0 : color;
}


// arch-tag: af19d9b6-7b4a-49ec-aee4-529be6aba253
