// grid.cc -- 2d sample generator using a simple grid
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "rand.h"

#include "grid.h"

using namespace snogray;

void
Grid::generate ()
{
  float v_offs = 0;

  clear ();

  for (unsigned i = 0; i < v_steps; i++)
    {
      float u_offs = 0;

      for (unsigned j = 0; j < u_steps; j++)
	{
	  if (jitter)
	    add (u_offs + random (u_step), v_offs + random (v_step));
	  else
	    add (u_offs, v_offs);

	  u_offs += u_step;
	}

      v_offs += v_step;
    }
}

// arch-tag: f06e6561-bb1b-4fd0-982b-3120c89b8cf2
