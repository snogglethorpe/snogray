// path-integ.h -- Path-tracing surface integrator
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

#ifndef __PATH_INTEG_H__
#define __PATH_INTEG_H__

#include "surface-integ.h"
#include "direct-illum.h"


namespace snogray {


class PathInteg : public SurfaceInteg
{
public:

  // Global state for this integrator, for rendering an entire scene.
  //
  class GlobalState : public SurfaceInteg::GlobalState
  {
  public:

    GlobalState (const Scene &_scene, const ValTable &params);

    // Return a new integrator, allocated in context.
    //
    virtual SurfaceInteg *make_integrator (RenderContext &context);

  private:

    friend class PathInteg;

    // We will try to extend paths to at least this many vertices (unless
    // they fail strike any object in the scene).  Paths longer than this
    // will be terminated randomly using russian roulette.
    //
    // This parameter also controls the number of path vertices for which
    // we pre-calculate well-distributed sampling parameters; paths longer
    // than this use more randomly distributed samples.
    //
    unsigned min_path_len;

    // Probability we will terminate a path at each vertex beyond the
    // MIN_PATH_LEN.
    //
    float russian_roulette_terminate_probability;

    // Global state for DirectIllum objects.
    //
    DirectIllum::GlobalState direct_illum;
  };

  // Return the light arriving at RAY's origin from the direction it
  // points in (the length of RAY is ignored).  MEDIA is the media
  // environment through which the ray travels.
  //
  // This method also calls the volume-integrator's Li method, and
  // includes any light it returns for RAY as well.
  //
  // "Li" means "Light incoming".
  //
  virtual Tint Li (const Ray &ray, const Media &media,
		   const SampleSet::Sample &sample);

private:

  // Integrator state for rendering a group of related samples.
  //
  PathInteg (RenderContext &context, GlobalState &global_state);

  // Pointer to our global state info.
  //
  const GlobalState &global;

  // Direct illumination objects used for the first SHORT_PATH_LEN path
  // vertices.
  //
  std::vector<DirectIllum> vertex_direct_illums;

  // BSDF sample-channels used for the first SHORT_PATH_LEN path vertices.
  //
  SampleSet::ChannelVec<UV> bsdf_sample_channels;

  //
  // The RANDOM_DIRECT_ILLUM and RANDOM_SAMPLE_SET fields are modified by
  // PathInteg::Li, but their state need not be preserved between calls.
  //
  // The reason they are fields in the PathInteg object rather than being
  // local variables in PathInteg::Li is to save unnecessary initialization
  // time and memory allocation in PathInteg::Li, which is called once per
  // eye-ray.
  //
  // This means we have to be careful about reentrancy, but this is
  // manageable:  A given PathInteg object should only be used within a
  // single thread, and PathInteg::Li should only be called recursively
  // when it calls VolumeInteg::Li; such recursive usage should be fine, as
  // RANDOM_DIRECT_ILLUM and RANDOM_SAMPLE_SET are only actively in use
  // when computing direct-lighting.
  //

  // This is a special dedicated sample-set which we use just for
  // RANDOM_DIRECT_ILLUM.
  //
  SampleSet random_sample_set;

  // DirectIllum object used to do direct illumination for path vertices
  // when the path-length is greater than MIN_PATH_LEN.
  //
  DirectIllum random_direct_illum;
};


}

#endif // __PATH_INTEG_H__
