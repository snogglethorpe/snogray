// render-stats.h -- Print post-rendering statistics
//
//  Copyright (C) 2005, 2006, 2007, 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_RENDER_STATS_H
#define SNOGRAY_RENDER_STATS_H

#include <iosfwd>


namespace snogray {


class Scene;

struct RenderStats
{
  RenderStats ()
    : scene_intersect_calls (0), scene_shadow_tests (0), illum_calls (0)
  { }

  struct IsecStats
  {
    IsecStats ()
      : surface_intersects_tests (0), surface_intersects_hits (0),
	neg_cache_hits (0), neg_cache_collisions (0),
	space_node_intersect_calls (0)
    { }

    void operator+= (const IsecStats &is)
    {
      surface_intersects_tests += is.surface_intersects_tests;
      surface_intersects_hits += is.surface_intersects_hits;
      neg_cache_hits += is.neg_cache_hits;
      neg_cache_collisions += is.neg_cache_collisions;
      space_node_intersect_calls += is.space_node_intersect_calls;
    }

    unsigned long long surface_intersects_tests;
    unsigned long long surface_intersects_hits;
    unsigned long long neg_cache_hits;
    unsigned long long neg_cache_collisions;
    unsigned long long space_node_intersect_calls;
  };

  void operator+= (const RenderStats &is)
  {
    scene_intersect_calls += is.scene_intersect_calls;
    scene_shadow_tests += is.scene_shadow_tests;
    illum_calls += is.illum_calls;

    intersect += is.intersect;
    shadow += is.shadow;
  }

  unsigned long long scene_intersect_calls;
  unsigned long long scene_shadow_tests;
  unsigned long long illum_calls;
  
  IsecStats intersect, shadow;

  void print (std::ostream &os);
};

}

#endif /*SNOGRAY_RENDER_STATS_H */

// arch-tag: b7800699-80ca-46da-9f30-732a78beb547
