// space.cc -- Space-division abstraction (hierarchically arranges 3D space)
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "space.h"

using namespace Snogray;

Space::~Space () { }				    // stop gcc bitching
Space::IntersectCallback::~IntersectCallback () { } // stop gcc bitching

// arch-tag: 7a5f3914-e339-4d52-b400-e3c713c5aa9a
