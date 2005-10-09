// transform.cc -- Transformation matrices
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "transform.h"

using namespace Snogray;

Transform
Transform::operator* (const Transform &xform)
{
  Transform result;
  for (unsigned i = 0; i < 4; i++)
    for (unsigned j = 0; j < 4; j++)
      result(i, j)
	= els[i][0] * xform (0, j)
	+ els[i][1] * xform (1, j)
	+ els[i][2] * xform (2, j)
	+ els[i][3] * xform (3, j);
  return result;
}

// arch-tag: 95232aab-70c4-4506-a8c0-2705aba6ba88
