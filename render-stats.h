// render-stats.h -- Print post-rendering statistics
//
//  Copyright (C) 2005, 2006, 2007, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __RENDER_STATS_H__
#define __RENDER_STATS_H__

#include <ostream>


namespace snogray {


class Scene;

struct RenderStats
{
  RenderStats ()
    : scene_intersect_calls (0),
      scene_shadow_tests (0), shadow_hint_hits (0), shadow_hint_misses (0),
      scene_slow_shadow_traces (0), surface_slow_shadow_traces (0),
      horizon_hint_hits (0), horizon_hint_misses (0),
      illum_calls (0), illum_samples (0), illum_specular_samples (0)
  { }

  struct IsecStats
  {
    IsecStats ()
      : surface_intersects_tests (0), surface_intersects_hits (0),
	neg_cache_hits (0), neg_cache_collisions (0),
	space_node_intersect_calls (0)
    { }

    unsigned long long surface_intersects_tests;
    unsigned long long surface_intersects_hits;
    unsigned long long neg_cache_hits;
    unsigned long long neg_cache_collisions;
    unsigned long long space_node_intersect_calls;
  };

  unsigned long long scene_intersect_calls;
  unsigned long long scene_shadow_tests;
  unsigned long long shadow_hint_hits;
  unsigned long long shadow_hint_misses;
  unsigned long long scene_slow_shadow_traces;
  unsigned long long surface_slow_shadow_traces;
  unsigned long long horizon_hint_hits;
  unsigned long long horizon_hint_misses;
  unsigned long long illum_calls;
  unsigned long long illum_samples;
  unsigned long long illum_specular_samples;
  
  IsecStats intersect, shadow;

  void print (std::ostream &os);
};

}

#endif /*__RENDER_STATS_H__ */

// arch-tag: b7800699-80ca-46da-9f30-732a78beb547
