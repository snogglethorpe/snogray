// sample-set.cc -- Set of samples
//
//  Copyright (C) 2010, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "util/random.h"

#include "sample-set.h"

using namespace snogray;


// Removes all samples from this sample-set, invalidating any previously
// created channels.  To subsequently generate more samples, new channels
// must be added.
//
void
SampleSet::clear ()
{
  float_samples.clear ();
  uv_samples.clear ();

  float_channels.clear ();
  uv_channels.clear ();
}

// Compute a completely new set of sample values in all channels.
//
void
SampleSet::generate ()
{
  for (std::vector<Channel<float> >::iterator i = float_channels.begin();
       i != float_channels.end (); ++i)
    if (i->num_total_samples != 0)
      {
	std::vector<float>::iterator base = sample<float> (i->base_offset);
	gen.gen_samples<float> (random, base, i->num_total_samples);
	random_shuffle (base, base + i->num_total_samples, random);
      }

  for (std::vector<Channel<UV> >::iterator i = uv_channels.begin();
       i != uv_channels.end (); ++i)
    if (i->num_total_samples != 0)
      {
	std::vector<UV>::iterator base = sample<UV> (i->base_offset);
	gen.gen_samples<UV> (random, base, i->num_total_samples);
	random_shuffle (base, base + i->num_total_samples, random);
      }
}


// Add enough entries to the end of our sample table for samples of
// type T to hold NUM_SAMPLES samples, and return the offset of the
// first entry so allocated.
//
// These specializations are out-of-line to try and avoid bloat a little bit.

template<>
unsigned
SampleSet::add_sample_space<float> (unsigned num_samples)
{
  unsigned base_offset = float_samples.size ();
  float_samples.resize (base_offset + num_samples);
  return base_offset;
}

template<>
unsigned
SampleSet::add_sample_space<UV> (unsigned num_samples)
{
  unsigned base_offset = uv_samples.size ();
  uv_samples.resize (base_offset + num_samples);
  return base_offset;
}

