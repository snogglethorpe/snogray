// mitchell.cc -- Mitchell filter
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "mitchell.h"

using namespace Snogray;


float
Mitchell::val (float x, float y) const
{
  return mitchell1 (x * inv_x_width) * mitchell1 (y * inv_y_width);
}

// arch-tag: ce717d07-2de4-46ce-bf7a-49d049c9e32c
