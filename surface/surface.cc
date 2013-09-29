// surface.cc -- Physical surface
//
//  Copyright (C) 2005-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <stdexcept>

#include "space/space.h"
#include "space/space-builder.h"

#include "surface.h"


using namespace snogray;


// Return statistics about this surface (see the definition of
// Surface::Stats below for details).
//
Surface::Stats
Surface::stats () const
{
  Stats stats;
  StatsCache cache;

  accum_stats (stats, cache);

  return stats;
}



// Surface::Stats

Surface::Stats &
Surface::Stats::operator+= (const Stats &stats)
{
  num_render_surfaces += stats.num_render_surfaces;
  num_real_surfaces += stats.num_real_surfaces;
  num_lights += stats.num_lights;
  return *this;
}


// arch-tag: a62e1854-d7ca-4cb3-a8dc-9be328c53430
