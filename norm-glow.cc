// norm-glow.cc -- Material whose color indicates surface normal
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "intersect.h"

#include "norm-glow.h"

using namespace snogray;

Color
NormGlow::render (const Intersect &isec) const
{
  return Color (intens * isec.n.x, intens * isec.n.y, intens * isec.n.z);
}
