// render-stats.cc -- Print post-rendering statistics
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

#include <iostream>
#include <iomanip>

#include "scene.h"
#include "string-funs.h"

#include "render-stats.h"

using namespace snogray;
using namespace std;

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
RenderStats::print (ostream &os)
{
  long long sc  = scene_intersect_calls;
  long long tnc = intersect.space_node_intersect_calls;

  os << endl;
  os << "Rendering stats:" << endl;
  os << "  intersect:" << endl;
  os << "     rays:            " << setw (16) << commify (sc) << endl;
  os << "     tree node tests: " << setw (16) << commify (tnc) << endl;

  {
    long long surf_tests = intersect.surface_intersects_tests;
    long long neg_cache_hits = intersect.neg_cache_hits;
    long long neg_cache_colls = intersect.neg_cache_collisions;
    long long tot_tries = surf_tests + neg_cache_hits;
    long long pos_tries = intersect.surface_intersects_hits;

    os << "     surface tests:   " << setw (16) << commify (tot_tries)
       << " (success = " << setw(2) << percent (pos_tries, tot_tries)
       << "%, cached = " << setw(2) << percent (neg_cache_hits, tot_tries)
       << "%; coll = "<< setw(2) << percent (neg_cache_colls, tot_tries)
       << "%)"
       << endl;
  }

  long long sst = scene_shadow_tests;

  if (sst != 0)
    {
      long long sss = scene_slow_shadow_traces;
      long long oss = surface_slow_shadow_traces;
      long long tnt = shadow.space_node_intersect_calls;

      os << "  shadow:" << endl;
      os << "     rays:            " << setw (16) << commify (sst)
	 << endl;
      if (sss != 0)
	os << "     non-opaque traces: " << setw (14) << commify (sss)
	   << " (" << setw(2) << percent (sss, sst) << "%"
	   << "; average depth = " << fraction (oss, sss) << ")"
	   << endl;
      os << "     tree node tests: " << setw (16) << commify (tnt) << endl;

      {
	long long surf_tests  = shadow.surface_intersects_tests;
	long long neg_cache_hits = shadow.neg_cache_hits;
	long long neg_cache_colls = shadow.neg_cache_collisions;
	long long tot_tries = surf_tests + neg_cache_hits;
	long long pos_tries = shadow.surface_intersects_hits;

	os << "     surface tests:   " << setw (16) << commify (tot_tries)
	   << " (success = " << setw(2) << percent (pos_tries, tot_tries)
	   << "%, cached = " << setw(2) << percent (neg_cache_hits, tot_tries)
	   << "%; coll = "<< setw(2) << percent (neg_cache_colls, tot_tries)
	   << "%)"
	   << endl;
      }
    }

  long long ic = illum_calls;

  if (ic != 0)
    {
      long long isi = illum_samples - illum_specular_samples;
      long long iss = illum_specular_samples;

      os << "  illum:" << endl;
      os << "     illum calls:     " << setw (16)
	 << commify (ic) << endl;
      if (isi)
	os << "     average non-spec samples:" << setw (8)
	   << setprecision(3) << fraction (isi, ic) << endl;
      if (iss)
	os << "     average specular samples:" << setw (8)
	   << setprecision(3) << fraction (iss, ic) << endl;
      if (sst)
	os << "     average shadow rays:   " << setw (10)
	   << setprecision(3) << fraction (sst, ic)
	   << " (" << setw(2) << percent (sst, isi) << "%)" << endl;
    }
}

// arch-tag: b884b170-54ff-4f69-a847-0997e0b0f347
