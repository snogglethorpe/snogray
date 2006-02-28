// lsamples.h -- Samples of light sources irradiating a surface point
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __LSAMPLES_H__
#define __LSAMPLES_H__

#include <vector>

#include "light.h"

namespace Snogray {

class Brdf;
class Color;
class Intersect;
class Trace;

class LightSamples
{
public:

  void generate (const Intersect &isec, const std::vector<Light *> lights);

  // Accessors for the final result.
  //
  unsigned size () const
  {
    return sample_indices.size ();
  }
  const SampleRay &operator[] (unsigned i) const
  {
    return samples[sample_indices[i]];
  }
  SampleRay &operator[] (unsigned i)
  {
    return samples[sample_indices[i]];
  }

protected:

  SampleRayVec samples;

  // Final samples
  //
  std::vector<unsigned> sample_indices;

  // Common iterator definition
  //
  template<typename LS, typename V, typename I>
  class Iter
  {
  public:

    typedef V value_type;
    typedef typename I::difference_type difference_type;
    typedef typename I::iterator_category iterator_category;
    typedef V *pointer;
    typedef V &reference;

    Iter (LS &_lsamples, I _idx_iter)
      : lsamples (_lsamples), idx_iter (_idx_iter)
    { }

    V &operator* () const { return lsamples.samples[*idx_iter]; }
    V *operator-> () const { return &lsamples.samples[*idx_iter]; }

    Iter &operator++ () { idx_iter++; return *this; }
    Iter operator++ (int) { Iter old = *this; idx_iter++; return old; }

    bool operator!= (const Iter &i) const { return idx_iter != i.idx_iter; }
    bool operator== (const Iter &i) const { return idx_iter == i.idx_iter; }

    bool operator- (const Iter &i) const { return idx_iter - i.idx_iter; }

    void operator += (difference_type d) { idx_iter += d; }

    operator unsigned () const { return *idx_iter; }

  private:

    LS &lsamples;

    I idx_iter;
  };

public:

  // Iterator for the final result
  //
  typedef Iter<LightSamples, SampleRay, std::vector<unsigned>::iterator>
    iterator;
  iterator begin () { return iterator (*this, sample_indices.begin ()); }
  iterator end () { return iterator (*this, sample_indices.end ()); }

  // const-iterator for the final result
  //
  typedef Iter<const LightSamples, const SampleRay,
	       std::vector<unsigned>::const_iterator>
    const_iterator;
  const_iterator begin () const
  {
    return const_iterator (*this, sample_indices.begin ());
  }
  const_iterator end () const
  {
    return const_iterator (*this, sample_indices.end ());
  }
};

}

#endif /* __LSAMPLES_H__ */

// arch-tag: 14039340-97f4-4468-ba57-27339cfde704
