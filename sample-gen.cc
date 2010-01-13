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
std::vector<float> *
SampleGen::gen_shuffled_samples ()
{
  std::vector<float> *table = gen_samples<float> ();
  random_shuffle (table->begin (), table->end ());
  return table;
}

template<>
std::vector<UV> *
SampleGen::gen_shuffled_samples ()
{
  std::vector<UV> *table = gen_samples<UV> ();
  random_shuffle (table->begin (), table->end ());
  return table;
}
