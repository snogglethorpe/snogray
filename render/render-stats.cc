// render-stats.cc -- Print post-rendering statistics
//
//  Copyright (C) 2005-2007, 2010, 2012, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>
#include <iomanip>

#include "scene/scene.h"
#include "util/string-funs.h"

#include "render-stats.h"

using namespace snogray;


// Return 100 * (NUM / DEN) as an int; if DEN == 0, return 0.
//
static int percent (long long num, long long den)
{
  return den == 0 ? 0 : (100 * num / den);
}

// Return NUM / DEN as a float; if DEN == 0, return 0;
//
static float fraction (long long num, long long den)
{
  return den == 0 ? 0 : (float (num) / float (den));
}

// Print post-rendering scene statistics
//
void
RenderStats::print (std::ostream &os)
{
  long long sc  = scene_intersect_calls;
  long long tnc = intersect.space_node_intersect_calls;

  os << std::endl;
  os << "Rendering stats:" << std::endl;
  os << "  intersect:" << std::endl;
  os << "     rays:            " << std::setw (16) << commify (sc) << std::endl;
  os << "     tree node tests: " << std::setw (16) << commify (tnc) << std::endl;

  {
    long long surf_tests = intersect.surface_intersects_tests;
    long long neg_cache_hits = intersect.neg_cache_hits;
    long long neg_cache_colls = intersect.neg_cache_collisions;
    long long tot_tries = surf_tests + neg_cache_hits;
    long long pos_tries = intersect.surface_intersects_hits;

    os << "     surface tests:   " << std::setw (16) << commify (tot_tries)
       << " (success = " << std::setw(2) << percent (pos_tries, tot_tries)
       << "%, cached = " << std::setw(2) << percent (neg_cache_hits, tot_tries)
       << "%; coll = "<< std::setw(2) << percent (neg_cache_colls, tot_tries)
       << "%)"
       << std::endl;
  }

  long long sst = scene_shadow_tests;

  if (sst != 0)
    {
      long long tnt = shadow.space_node_intersect_calls;

      os << "  shadow:" << std::endl;
      os << "     rays:            "
	 << std::setw (16) << commify (sst) << std::endl;
      os << "     tree node tests: "
	 << std::setw (16) << commify (tnt) << std::endl;

      {
	long long surf_tests  = shadow.surface_intersects_tests;
	long long neg_cache_hits = shadow.neg_cache_hits;
	long long neg_cache_colls = shadow.neg_cache_collisions;
	long long tot_tries = surf_tests + neg_cache_hits;
	long long pos_tries = shadow.surface_intersects_hits;

	os << "     surface tests:   " << std::setw (16) << commify (tot_tries)
	   << " (success = " << std::setw(2) << percent (pos_tries, tot_tries)
	   << "%, cached = "
	   << std::setw(2) << percent (neg_cache_hits, tot_tries)
	   << "%; coll = "
	   << std::setw(2) << percent (neg_cache_colls, tot_tries)
	   << "%)"
	   << std::endl;
      }
    }

  long long ic = illum_calls;

  if (ic != 0)
    {
      os << "  illum:" << std::endl;
      os << "     illum calls:     " << std::setw (16)
	 << commify (ic) << std::endl;
      if (sst)
	os << "     average shadow rays:   " << std::setw (10)
	   << std::setprecision(3) << fraction (sst, ic) << std::endl;
    }
}

// arch-tag: b884b170-54ff-4f69-a847-0997e0b0f347
