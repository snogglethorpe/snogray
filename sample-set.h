// sample-set.h -- Set of samples
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

#ifndef __SAMPLE_SET_H__
#define __SAMPLE_SET_H__

#include <vector>

#include "sample-gen.h"


namespace snogray {


// A set of samples.  There are zero or more channels, each holding the
// same number of samples.  Each channel has samples generated by the same
// generator, but the channels are explicitly de-correlated from each other
// by randomly shuffling the samples in each channel after generation.
//
class SampleSet
{
public:

  // A single sample channel.  Sample channels are typed, so they can only
  // contain a single type of sample (the available types of samples are
  // restricted to whatever the sample generator can generate).
  //
  template<typename T>
  class Channel
  {
  public:

    // Default constructor zero-initializes, hopefully resulting in a
    // segfault if an otherwise uninitialized channel is used by mistake.
    //
    Channel () {}

    // Copy constructor
    //
    Channel (const Channel &from)
      : size (from.size), base_offset (from.base_offset)
    {}

    // Number of nsub-samples this channel contains.  There are this many
    // sub-samples per top-level sample.
    //
    unsigned size;

  private:

    friend class SampleSet;

    // Normal constructor.  This is private, as BASE_OFFSET is an
    // implementation detail.
    //
    Channel (unsigned _base_offset, unsigned _size)
      : size (_size), base_offset (_base_offset)
    {}

    // Offset of our first sample in the appropriate sample vector of
    // our SampleSet.
    //
    unsigned base_offset;
  };

  // A vector of channels, for cases where we need more than one.
  //
  template<typename T>
  struct ChannelVec : public std::vector<Channel<T> >
  {
  };


  // Construct a new sample set, using the specified sample generator.
  //
  SampleSet (unsigned _num_samples, SampleGen &_gen)
    : num_samples (_num_samples), gen (_gen)
  {}


  // Return sample for top-level sample SAMPLE_NUM, and sub-sample
  // SUB_SAMPLE_NUM, from the sample channel CHANNEL.  SUB_SAMPLE_NUM
  // may be omitted if there's only one sample per top-level sample.
  //
  template<typename T>
  T get (const Channel<T> &channel,
	 unsigned sample_num, unsigned sub_sample_num = 0)
  {
    return sample<T> (channel.base_offset)
      [sample_num * channel.size + sub_sample_num];
  }

  // Return an iterator pointing to the first sub-sample for top-level
  // sample SAMPLE_NUM from the sample channel CHANNEL.
  //
  template<typename T>
  typename std::vector<T>::iterator
  begin (const Channel<T> &channel, unsigned sample_num) const
  {
    return sample<T> (channel.base_offset) + (sample_num * channel.size);
  }

  // Return an iterator pointing just past the end of the last
  // sub-sample for top-level sample SAMPLE_NUM from the sample channel
  // CHANNEL.
  //
  template<typename T>
  typename std::vector<T>::iterator
  end (const Channel<T> &channel, unsigned sample_num) const
  {
    return begin (channel, sample_num) + channel.size;
  }

  // Allocate a new sample-channel in this set, containing
  // NUM_SUB_SAMPLES samples per top-level sample (which defaults to 1).
  // The type of sample must be specified as the first template
  // parameter.
  //
  template<typename T>
  Channel<T> add_channel (unsigned num_sub_samples = 1)
  {
    // There's NUM_SUB_SAMPLES per top-level sample, so calculate the
    // total number of samples for this channel.
    //
    unsigned num_total_samples = num_samples * num_sub_samples;

    // Some sample generators may want a slightly different number of
    // samples.
    //
    num_total_samples = gen.adjust_sample_count<T> (num_total_samples);

    // Add enough room to our sample array for all the samples.
    //
    unsigned base_sample_offset = add_sample_space<T> (num_total_samples);

    // Generate (and shuffle) the actual samples.
    //
    gen.gen_shuffled_samples<T> (sample<T> (base_sample_offset),
				 num_total_samples);

    return Channel<T> (base_sample_offset, num_sub_samples);
  }

  // Allocate and return a vector of channels in this set, each
  // containing NUM_SUB_SAMPLES samples per top-level sample.  The type
  // of sample must be specified as the first template parameter.
  //
  template<typename T>
  ChannelVec<T> add_channel_vec (unsigned size, unsigned num_sub_samples)
  {
    ChannelVec<T> vec (size);
    for (unsigned i = 0; i < size; i++)
      vec[i] = add_channel<T> (num_sub_samples);
    return vec;
  }

  // Removes all samples from this sample-set, invalidating any previously
  // created channels.  To subsequently generate more samples, new channels
  // must be added.
  //
  void clear ()
  {
    float_samples.clear ();
    uv_samples.clear ();
  }

  // Number of top-level samples.
  //
  unsigned num_samples;

private:

  // Returns an iterator pointing into sample space for samples of type
  // T at offset OFFSET.
  //
  template<typename T>
  const typename std::vector<T>::iterator sample (unsigned offset);
  
  // Add enough entries to the end of our sample table for samples of
  // type T to hold NUM samples, and return the offset of the
  // first entry so allocated.
  //
  template<typename T>
  unsigned add_sample_space (unsigned num);

  std::vector<float> float_samples;
  std::vector<UV> uv_samples;

  SampleGen &gen;
};


//
// Declarations for specialized SampleSet::add_sample_space methods.
//

template<>
unsigned
SampleSet::add_sample_space<float> (unsigned num_samples);

template<>
unsigned
SampleSet::add_sample_space<UV> (unsigned num_samples);

//
// Specializations of SampleGen::base_sample for supported sample types.
//

template<>
inline const std::vector<float>::iterator
SampleSet::sample<float> (unsigned offset)
{
  return float_samples.begin() + offset;
}

template<>
inline const std::vector<UV>::iterator
SampleSet::sample<UV> (unsigned offset)
{
  return uv_samples.begin() + offset;
}


}

#endif // __SAMPLE_SET_H__
