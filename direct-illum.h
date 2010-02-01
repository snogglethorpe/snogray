// direct-illum.h -- Direct-lighting calculations
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

#ifndef __DIRECT_ILLUM_H__
#define __DIRECT_ILLUM_H__

#include "color.h"
#include "sample-set.h"


namespace snogray {


class Intersect;
class ValTable;
class Light;


class DirectIllum
{
public:

  // Global state for this illumrator, for rendering an entire scene.
  //
  class GlobalState
  {
  public:

    GlobalState (const ValTable &params);

  private:

    unsigned num_light_samples;

    friend class DirectIllum;
  };

  DirectIllum (RenderContext &context, const GlobalState &global_state);

  // Given an intersection resulting from a cast ray, sample lights
  // in the scene, and return their contribution in that
  // ray's direction.
  //
  Color sample_lights (const Intersect &isec, const SampleSet::Sample &sample)
    const
  {
    // XXX  For now, just do all lights.  In the future we should add a way
    // to limit the number of light samples in the case where there are
    // many lights (e.g., divide the desired number of light samples among
    // lights in the scene).
    //
    return sample_all_lights (isec, sample);
  }

  // Given an intersection resulting from a cast ray, sample all lights
  // in the scene, and return the sum of their contribution in that
  // ray's direction.
  //
  Color sample_all_lights (const Intersect &isec,
			   const SampleSet::Sample &sample)
    const;

  // Use multiple-importance-sampling to estimate the radiance of LIGHT
  // towards ISEC, using LIGHT_PARAM, BRDF_PARAM, and BRDF_LAYER_PARAM
  // to sample both the light and the BRDF.
  //
  Color sample_light (const Intersect &isec, const Light *light,
		      const UV &light_param,
		      const UV &brdf_param, float brdf_layer_param)
    const;

private:

  // Sample channels for light sampling.
  //
  SampleSet::ChannelVec<UV> light_samp_channels;
  SampleSet::Channel<float> light_select_chan;

  // Sample channels for brdf sampling.
  //
  SampleSet::ChannelVec<UV> brdf_samp_channels;
  SampleSet::ChannelVec<float> brdf_layer_channels;
};


}

#endif // __DIRECT_ILLUM_H__
