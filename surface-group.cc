// surface-group.cc -- Group of surfaces
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "surface-group.h"


using namespace snogray;


// Add this (or some other ...) surfaces to SPACE
//
void
SurfaceGroup::add_to_space (Space *space)
{
  for (std::list<Surface *>::const_iterator si = surfaces.begin();
       si != surfaces.end(); ++si)
    (*si)->add_to_space (space);
}
