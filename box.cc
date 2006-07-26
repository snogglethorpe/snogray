// box.cc -- Boxian filter
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "box.h"

using namespace Snogray;


float
Box::val (float, float) const
{
  return 1;
}

// arch-tag: 1f16abed-956e-4103-8459-ccec39283138
