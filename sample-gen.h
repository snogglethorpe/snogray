// sample-gen.h -- Sample generator
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

#ifndef __SAMPLE_GEN_H__
#define __SAMPLE_GEN_H__

#include <vector>

#include "uv.h"


namespace snogray {


// A sample generator, which can generate a specified number of samples to
// cover a certain number of dimensions "evenly".
//
// This class is defined in generically using a templates, but only certain
// types of samples are supported:  float, UV
//
class SampleGen
{
public:

  template<typename T>
  std::vector<T> *gen_shuffled_samples ();

  template<typename T>
  std::vector<T> *gen_samples ();

protected:

  SampleGen (unsigned _num_samples) : num_samples (_num_samples) { }

  // The actual sample generating methods, defined by subclasses.
  //
  virtual std::vector<float> *gen_float_samples () = 0;
  virtual std::vector<UV> *gen_uv_samples () = 0;

public:

  // The number of samples this generator will generate.
  //
  unsigned num_samples;
};


//
// Declarations of SampleGen::gen_shuffled_samples for supported sample types.
//

template<>
std::vector<float> *SampleGen::gen_shuffled_samples ();

template<>
std::vector<UV> *SampleGen::gen_shuffled_samples ();


//
// Specializations of SampleGen::gen_samples for supported sample types.
//

template<>
inline std::vector<float> *
SampleGen::gen_samples ()
{
  return gen_float_samples ();
}

template<>
inline std::vector<UV> *
SampleGen::gen_samples ()
{
  return gen_uv_samples ();
}


}

#endif // __SAMPLE_GEN_H__
