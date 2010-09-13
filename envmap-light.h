// envmap-light.v_sz -- Abstract class for envmapured light sources
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

#ifndef __ENVMAP_LIGHT_H__
#define __ENVMAP_LIGHT_H__


#include "light.h"
#include "envmap.h"
#include "frame.h"
#include "hist-2d.h"
#include "hist-2d-dist.h"


namespace snogray {


class EnvmapLight : public Light
{
public:

  EnvmapLight (const Ref<Envmap> &_envmap, const Frame &_frame = Frame ());

  // Return a sample of this light from the viewpoint of ISEC (using a
  // surface-normal coordinate system, where the surface normal is
  // (0,0,1)), based on the parameter PARAM.
  //
  virtual Sample sample (const Intersect &isec, const UV &param) const;

  // Return a "free sample" of this light.
  //
  virtual FreeSample sample (const UV &param, const UV &dir_param) const;

  // Evaluate this light in direction DIR from the viewpoint of ISEC (using
  // a surface-normal coordinate system, where the surface normal is
  // (0,0,1)).
  //
  virtual Value eval (const Intersect &isec, const Vec &dir) const;

  // Return true if this is an "environmental" light, not associated
  // with any surface.
  //
  virtual bool is_environ_light () const { return true; }

  // Evaluate this environmental light in direction DIR (in world-coordinates).
  //
  virtual Color eval_environ (const Vec &dir) const
  {
    return envmap->map (frame.to (dir));
  }

  // Do any scene-related setup for this light.  This is is called once
  // after the entire scene has been loaded.
  //
  virtual void scene_setup (const Scene &scene);

private:

  // Return a 2d histogram containing the intensity of ENVMAP, with the
  // intensity adusted to reflect the area distortion caused by mapping
  // it to a sphere.
  //
  Hist2d envmap_histogram (const Ref<Envmap> &envmap);

  Ref<Envmap> envmap;

  // Frame of reference for the environment map.
  //
  Frame frame;

  // Distribution for sampling the intensity of ENVMAP.
  //
  Hist2dDist intensity_dist;

  // Center and radius of a bounding sphere for the engire scene.
  //
  Pos scene_center;
  dist_t scene_radius;
};


}

#endif /* __ENVMAP_LIGHT_H__ */
