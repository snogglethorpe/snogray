// envmap-light.v_sz -- Abstract class for envmapured light sources
//
//  Copyright (C) 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_ENVMAP_LIGHT_H
#define SNOGRAY_ENVMAP_LIGHT_H

#include "texture/envmap.h"
#include "geometry/frame.h"

#include "light.h"


namespace snogray {


class EnvmapLight : public Light
{
public:

  EnvmapLight (const Ref<Envmap> &_envmap, const Frame &_frame = Frame ())
    : envmap (_envmap), frame (_frame)
  { }

  // Add light-samplers for this light in SCENE to SAMPLERS.  Any
  // samplers added become owned by the owner of SAMPLERS, and will be
  // destroyed when it is.
  //
  virtual void add_light_samplers (
		 const Scene &scene,
		 std::vector<const Light::Sampler *> &samplers)
    const;

  // Transform the geometry of this light by XFORM.
  //
  virtual void transform (const Xform &xform);


private:

  class Sampler;

  Ref<Envmap> envmap;

  // Frame of reference for the environment map.
  //
  Frame frame;
};


}

#endif /* SNOGRAY_ENVMAP_LIGHT_H */
