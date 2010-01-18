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

  // Generate NUM shuffled samples and store them in TABLE.
  //
  template<typename T>
  void gen_shuffled_samples (const typename std::vector<T>::iterator &table,
			     unsigned num)
    const;

  // Generate NUM samples and store them in TABLE.
  //
  template<typename T>
  void gen_samples (const typename std::vector<T>::iterator &table,
		    unsigned num)
    const;

  // Return the number of samples we'd like to generate instead of NUM.
  //
  template<typename T>
  unsigned adjust_sample_count (unsigned num) const;

protected:

  // The actual sample generating methods, defined by subclasses.
  //
  virtual void gen_float_samples (const std::vector<float>::iterator &table,
				  unsigned num)
    const = 0;
  virtual void gen_uv_samples (const std::vector<UV>::iterator &table,
			       unsigned num)
    const = 0;

  // Sample-count adjusting methods defined by subclasses.  By default,
  // NUM is returned unchanged.
  //
  virtual unsigned adjust_float_sample_count (unsigned num) const
  {
    return num;
  }
  virtual unsigned adjust_uv_sample_count (unsigned num) const
  {
    return num;
  }
};


//
// Declarations of SampleGen::gen_shuffled_samples for supported sample types.
//

template<>
void
SampleGen::gen_shuffled_samples<float> (const std::vector<float>::iterator &table,
					unsigned num)
  const;

template<>
void
SampleGen::gen_shuffled_samples<UV> (const std::vector<UV>::iterator &table,
				     unsigned num)
  const;


//
// Specializations of SampleGen::gen_samples for supported sample types.
//

template<>
inline void
SampleGen::gen_samples<float> (const std::vector<float>::iterator &table,
			       unsigned num)
  const
{
  gen_float_samples (table, num);
}

template<>
inline void
SampleGen::gen_samples<UV> (const std::vector<UV>::iterator &table,
			    unsigned num)
  const
{
  gen_uv_samples (table, num);
}


//
// Specializations of SampleGen::adjust_sample_count for supported
// sample types.
//

template<>
inline unsigned
SampleGen::adjust_sample_count<float> (unsigned num) const
{
  return adjust_float_sample_count (num);
}

template<>
inline unsigned
SampleGen::adjust_sample_count<UV> (unsigned num) const
{
  return adjust_uv_sample_count (num);
}

}

#endif // __SAMPLE_GEN_H__
