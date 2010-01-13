// grid.h -- sample generator using a simple jittered grid
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

#ifndef __GRID_H__
#define __GRID_H__

#include "sample-gen.h"


namespace snogray {


class Grid : public SampleGen
{
public:

  Grid (unsigned num_samples);

protected:

  // The actual sample generating methods.
  //
  virtual std::vector<float> *gen_float_samples ();
  virtual std::vector<UV> *gen_uv_samples ();

private:

  unsigned u_steps, v_steps;

  // Step size used for U and V dimensions in UV samples.
  //
  float u_step, v_step;

  // Step size used for float samples.
  //
  float n_step;
};


}

#endif /* __GRID_H__ */

// arch-tag: 922e8ac5-56ed-425f-80ab-2acd5092b203
