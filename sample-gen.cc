// sample-gen.cc -- Sample generator
//
//  Copyright (C) 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <algorithm>

#include "sample-gen.h"


using namespace snogray;


//
// Specializations of SampleGen::gen_shuffled_samples for supported sample
// types.
//

template<>
void
SampleGen::gen_shuffled_samples<float> (const std::vector<float>::iterator &table,
					unsigned num)
  const
{
  gen_samples<float> (table, num);
  random_shuffle (table, table + num);
}

template<>
void
SampleGen::gen_shuffled_samples<UV> (const std::vector<UV>::iterator &table,
				     unsigned num)
  const
{
  gen_samples<UV> (table, num);
  random_shuffle (table, table + num);
}
