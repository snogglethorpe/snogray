// direct-illum.h -- Direct illuminator
//
//  Copyright (C) 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __DIRECT_ILLUM_H__
#define __DIRECT_ILLUM_H__

#include <vector>

#include "illum.h"
#include "intersect.h"


namespace snogray {

class Scene;
class Trace;
class IllumMgr;


class DirectIllum : public Illum
{
public:

  DirectIllum (const Scene &_scene, unsigned _uses)
    : Illum (_uses), scene (_scene)
  { }

  virtual ~DirectIllum () { }

  // Do shadow testing on the samples from BEG to END, and set their value
  // to zero if shadowed.
  //
  void shadow_test (const Intersect &isec,
		    IllumSampleVec::iterator beg, IllumSampleVec::iterator end)
    const;

protected:

  struct LightParams
  {
    LightParams () : num_samples (0), weight (0), is_point_light (false) { }

    // The number of light samples to (try to) generate.
    //
    unsigned num_samples;

    // Weight applied to light.  The weights of all lights add to one.
    //
    float weight;

    bool is_point_light;
  };

  // The STL allocator used by LightParamsVec, which allocates from a Mempool.
  //
  typedef MempoolAlloc<LightParams> LightParamsVecAlloc;

  class LightParamsVec : public std::vector<LightParams, LightParamsVecAlloc>
  {
  public:

    LightParamsVec (unsigned num_samples,
		    const std::vector<const Light *> &lights,
		    const Intersect &isec);
  };

  const Scene &scene;
};


}

#endif /* __DIRECT_ILLUM_H__ */


// arch-tag: 8f0e4b8d-3a62-497d-8453-6dda9717342c
