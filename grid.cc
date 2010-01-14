// grid.cc -- sample generator using a simple jittered grid
//
//  Copyright (C) 2006, 2007, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "snogmath.h"
#include "rand.h"

#include "grid.h"


using namespace snogray;


Grid::Grid (unsigned num_samples)
  : SampleGen (num_samples)
{
  v_steps = sqrt (num_samples);
  if (v_steps == 0)
    v_steps = 1;

  u_steps = num_samples / v_steps;
  if (u_steps * v_steps < num_samples)
    u_steps++;

  u_step = 1 / float (u_steps);
  v_step = 1 / float (v_steps);

  n_step = 1 / float (num_samples);
}

std::vector<UV> *
Grid::gen_uv_samples ()
{
  float v_offs = 0;

  // We allocate enough space to actually hold the entire grid, even if
  // that's more than NUM_SAMPLES samples; when the result is shuffled,
  // that will end up essentially discarding a random set of samples.
  //
  // (The shuffling process looks at the returned vector size to know how
  // many entries there are).
  //
  std::vector<UV> *samples = new std::vector<UV> (u_steps * v_steps);

  std::vector<UV>::iterator cur_samp = samples->begin ();

  for (unsigned i = 0; i < v_steps; i++)
    {
      float u_offs = 0;

      for (unsigned j = 0; j < u_steps; j++)
	{
	  *cur_samp++ = UV (u_offs + random (u_step), v_offs + random (v_step));
	  u_offs += u_step;
	}

      v_offs += v_step;
    }

  return samples;
}

std::vector<float> *
Grid::gen_float_samples ()
{
  float offs = 0;

  std::vector<float> *samples = new std::vector<float> (num_samples);

  std::vector<float>::iterator cur_samp = samples->begin ();

  for (unsigned i = 0; i < num_samples; i++)
    *cur_samp++ = offs + random (n_step);

  return samples;
}


// arch-tag: f06e6561-bb1b-4fd0-982b-3120c89b8cf2
