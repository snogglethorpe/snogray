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
#include "random.h"

#include "grid.h"


using namespace snogray;


void
Grid::gen_uv_samples (Random &random,
		      const std::vector<UV>::iterator &table, unsigned num)
  const
{
  ASSERT (num != 0);

  // We choose sample dimensions which are close to the square-root of NUM,
  // but allow them to differ by 1 from each other, so long as the product
  // is greater than or equal to NUM.
  //
  double sqrt_num = sqrt (double (num));
  double up = ceil (sqrt_num);
  double down = floor (sqrt_num + 0.5f);

  unsigned u_steps = unsigned (up);
  unsigned v_steps = unsigned (down);

  // NUM should already have been adjusted by Grid::adjust_uv_sample_count,
  // so our calculations should produce an exact result.  If this assertion
  // fails, the likely cause is a previous failure to call
  // Grid::adjust_uv_sample_count.
  //
  ASSERT (u_steps * v_steps == num);

  float u_step = 1 / up;
  float v_step = 1 / down;

  float v_offs = 0;

  std::vector<UV>::iterator samp = table;
  for (unsigned i = 0; i < v_steps; i++)
    {
      float u_offs = 0;

      for (unsigned j = 0; j < u_steps; j++)
	{
	  *samp++ = UV (clamp01 (u_offs + random () * u_step),
			clamp01 (v_offs + random () * v_step));
	  u_offs += u_step;
	}

      v_offs += v_step;
    }
}

unsigned
Grid::adjust_uv_sample_count (unsigned num) const
{
  // We choose sample dimensions which are close to the square-root of NUM,
  // but allow them to differ by 1 from each other, so long as the product
  // is greater than or equal to NUM.
  //
  // [Converting to a single-precision float here potentially loses some
  // bits, but we don't care, as the a number large enough to be a problem
  // represents a absurd number of samples (23 bit mantissa =~ 8 million).]
  //
  float sqrt_num = sqrt (float (num));
  float up = ceil (sqrt_num);
  float down = floor (sqrt_num + 0.5f);
  return unsigned (up * down);
}

void
Grid::gen_float_samples (Random &random,
			 const std::vector<float>::iterator &table,
			 unsigned num)
  const
{
  float n_step = 1 / float (num);
  float offs = 0;

  for (unsigned i = 0; i < num; i++)
    table[i] = clamp01 (offs + random () * n_step);
}


// arch-tag: f06e6561-bb1b-4fd0-982b-3120c89b8cf2
