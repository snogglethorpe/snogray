// grid.h -- 2d sample generator using a simple grid
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __GRID_H__
#define __GRID_H__

#include "sample2-gen.h"

namespace Snogray {

class Grid : public Sample2Gen
{
public:

  Grid (unsigned _u_steps, unsigned _v_steps, bool _jitter)
    : Sample2Gen (_u_steps * _v_steps),
      u_steps (_u_steps), v_steps (_v_steps),
      u_step (1 / float (_u_steps)), v_step (1 / float (_v_steps)),
      jitter (_jitter)
  { }

  virtual void generate ();

  unsigned u_steps, v_steps;

  float u_step, v_step;

  bool jitter;
};

}

#endif /* __GRID_H__ */

// arch-tag: 922e8ac5-56ed-425f-80ab-2acd5092b203
