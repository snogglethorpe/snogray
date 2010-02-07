// random-boost.h -- Random number classes using boost::random
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

#ifndef __RANDOM_BOOST_H__
#define __RANDOM_BOOST_H__

#include <boost/random.hpp>


namespace snogray {


typedef boost::mt19937 RNG;

// A wrapper class that turns an unsigned-generating RNG into a RNG that
// generates floats in the range 0-1.
//
template<typename RNG>
class FloatRNGAdaptor
{
public:

  FloatRNGAdaptor (RNG &rng) : vg (rng, boost::uniform_real<float> ()) { }

  float operator() () { return vg (); }

private:

  boost::variate_generator<RNG &, boost::uniform_real<float> > vg;
};


}

#endif // __RANDOM_BOOST_H__
