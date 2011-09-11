// random-tr1.h -- Random number classes using C++0x TR1 std::random
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

#ifndef SNOGRAY_RANDOM_TR1_H
#define SNOGRAY_RANDOM_TR1_H

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

  FloatRNGAdaptor (RNG &rng)
    : vg (rng, std::uniform_real<float> ())
  { }

  float operator() () { return vg (); }

private:

  std::variate_generator<RNG &, std::uniform_real<float> > vg;
};


}

#endif // SNOGRAY_RANDOM_TR1_H
