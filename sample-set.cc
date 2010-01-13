// sample-set.cc -- Set of samples
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

#include "sample-set.h"

using namespace snogray;


SampleSet::~SampleSet ()
{
  for (std::vector<std::vector<float> *>::iterator i = float_tables.begin();
       i != float_tables.end(); ++i)
    delete *i;
  for (std::vector<std::vector<UV> *>::iterator i = uv_tables.begin();
       i != uv_tables.end(); ++i)
    delete *i;
}


// Just adds TABLE to our list of tables; out-of-line to avoid bloat.
//
template<>
void
SampleSet::add_table (std::vector<float> *table)
{
  float_tables.push_back (table);
}
template<>
void
SampleSet::add_table<UV> (std::vector<UV> *table)
{
  uv_tables.push_back (table);
}
