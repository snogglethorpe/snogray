// primary-obj.cc -- Standalone (non-embedded) objects
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "primary-obj.h"

using namespace Snogray;

// Returns the material this object is made from
//
const Material *
PrimaryObj::material () const
{
  return _material;
}

// arch-tag: abaab0ac-52c8-471e-b7b9-6e9c619ac60c
