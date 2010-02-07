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
protected:

  // The actual sample generating methods.  Using RANDOM as a source of
  // randomness, add NUM samples to TABLE through TABLE+NUM.
  //
  virtual void gen_float_samples (Random &random,
				  const std::vector<float>::iterator &table,
				  unsigned num)
    const;
  virtual void gen_uv_samples (Random &random,
			       const std::vector<UV>::iterator &table,
			       unsigned num)
    const;

  virtual unsigned adjust_uv_sample_count (unsigned num) const;
};


}

#endif /* __GRID_H__ */

// arch-tag: 922e8ac5-56ed-425f-80ab-2acd5092b203
