// sample2-gen.h -- 2d sample generator
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SAMPLE2_GEN_H__
#define __SAMPLE2_GEN_H__

#include <vector>
#include <algorithm>

#include "uv.h"

namespace snogray {

class Sample2Gen : public std::vector<UV>
{
public:

  virtual ~Sample2Gen () { }

  virtual Sample2Gen *clone () const = 0;

  virtual void generate () = 0;

  void clear () { cur_sample = 0; }
  void add (float u, float v) { (*this)[cur_sample++] = UV (u, v); }

  void shuffle () { random_shuffle (begin (), end ()); }

protected:

  Sample2Gen (unsigned num_samples)
    : cur_sample (0)
  { resize (num_samples, UV (0, 0)); }

private:

  unsigned cur_sample;
};

}

#endif /* __SAMPLE2_GEN_H__ */

// arch-tag: 6fa4ec0d-375e-44bc-9085-6170eab59f2a
