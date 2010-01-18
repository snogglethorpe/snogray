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
#include "snogassert.h"
#include "rand.h"

#include "grid.h"


using namespace snogray;


void
Grid::gen_uv_samples (const std::vector<UV>::iterator &table, unsigned num)
  const
{
  assert (num != 0, "bogus sample count to Grid::gen_uv_samples");

  unsigned v_steps = sqrt (num);
  unsigned u_steps = num / v_steps;

  // NUM should already have been adjusted by
  // Grid::adjust_uv_sample_count, so make sure it's sane.
  //
  assert (u_steps * v_steps == num,
	  "bogus sample count to Grid::gen_uv_samples");

  float u_step = 1 / float (u_steps);
  float v_step = 1 / float (v_steps);

  float v_offs = 0;

  std::vector<UV>::iterator samp = table;
  for (unsigned i = 0; i < v_steps; i++)
    {
      float u_offs = 0;

      for (unsigned j = 0; j < u_steps; j++)
	{
	  *samp++ = UV (u_offs + random (u_step), v_offs + random (v_step));
	  u_offs += u_step;
	}

      v_offs += v_step;
    }
}

unsigned
Grid::adjust_uv_sample_count (unsigned num) const
{
  unsigned v_steps = sqrt (num);
  if (v_steps == 0)
    v_steps = 1;

  unsigned u_steps = num / v_steps;
  if (u_steps * v_steps < num)
    u_steps++;

  return u_steps * v_steps;
}

void
Grid::gen_float_samples (const std::vector<float>::iterator &table,
			 unsigned num)
  const
{
  float n_step = 1 / float (num);
  float offs = 0;

  for (unsigned i = 0; i < num; i++)
    table[i] = offs + random (n_step);
}


// arch-tag: f06e6561-bb1b-4fd0-982b-3120c89b8cf2
