// random-c0x.h -- Random number classes using C++0x std::random
//
//  Copyright (C) 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_RANDOM_C0X_H
#define SNOGRAY_RANDOM_C0X_H

#include <random>


namespace snogray {


typedef std::mt19937 RNG;

// A wrapper class that turns an unsigned-generating RNG into a RNG that
// generates floats in the range 0-1.
//
template<typename RNG>
class FloatRNGAdaptor
{
public:

  FloatRNGAdaptor (RNG &_rng) : rng (_rng) { }

  float operator() () { return dist (rng); }

private:

  std::uniform_real_distribution<float> dist;
  RNG &rng;
};


}

#endif // SNOGRAY_RANDOM_C0X_H
