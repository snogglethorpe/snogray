// gauss.cc -- Gaussian filter
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "gauss.h"

using namespace Snogray;

Gauss::~Gauss () { } // stop gcc bitching

float
Gauss::val (float x, float y) const
{
  return gauss1 (x, x_exp) * gauss1 (y, y_exp);
}

// arch-tag: 1cf8a7b3-1f31-4663-b256-1367c4b0020a
