// glow.cc -- Constant-color lighting model
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>

#include "glow.h"

#include "intersect.h"

using namespace Snogray;

const Color
Glow::render (const Intersect &isec,
	      const Vec &light_dir, const Color &light_color)
  const
{
  return color;
}

// arch-tag: af19d9b6-7b4a-49ec-aee4-529be6aba253
