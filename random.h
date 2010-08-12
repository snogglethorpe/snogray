// random.h -- Random number classes
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

#ifndef __RANDOM_H__
#define __RANDOM_H__

#include "config.h"

// Based on our configuration, include an implementation-specific header
// file to define the classes "RNG" and "FloatRNGAdaptor<>"
//
#if USE_STD_RANDOM
#include "random-c0x.h"
#elif USE_STD_TR1_RANDOM
#include "random-tr1.h"
#elif USE_BOOST_RANDOM
#include "random-boost.h"
#else
#include "random-rand.h"
#endif


namespace snogray {


// A class representing a random number generator.
//
class Random
{
public:

  Random () : frng (rng) { }
  Random (unsigned seed) : rng (seed), frng (rng) { }

  // Return a random floating-point number in the range 0-1.  It isn't
  // defined whether the ends of the range are inclusive or exclusive,
  // so callers should be prepared to handle either case.
  //
  float operator() () { return frng (); }

  // Return a random integer in the range [0, N).
  //
  unsigned operator() (unsigned n) { return rng () % n; }

  // Base random number generator.
  //
  RNG rng;

  // Floating-point view of RNG.
  //
  FloatRNGAdaptor<RNG> frng;
};


}

#endif // __RANDOM_H__
