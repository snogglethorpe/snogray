// space.cc -- Space-division abstraction (hierarchically arranges 3D space)
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "surface.h"

#include "space.h"


using namespace snogray;


// This is located here because putting it in "space.h" causes recursive
// include problems (because "space.h" then must include "surface.h").
//
void
Space::add (Surface *surface)
{
  add (surface, surface->bbox ());
}

// arch-tag: 550f9905-7373-4008-9c4e-e939d931f01d
