// envmap-light.v_sz -- Abstract class for envmapured light sources
//
//  Copyright (C) 2006, 2007, 2010  Miles Bader <miles@gnu.org>
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


#include "struct-light.h"
#include "envmap.h"


namespace snogray {


class EnvmapLight : public StructLight
{
public:

  EnvmapLight (const Ref<Envmap> &_envmap);

  // Dump a picture of the generated light regions to a file called
  // FILENAME.  ENVMAP should be the original environment map with which
  // this light was created.
  //
  void dump (const std::string &filename, const Envmap &orig_envmap) const
  {
    StructLight::dump (filename, orig_envmap.light_map ());
  }

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

  // Do any scene-related setup for this light.  This is is called once
  // after the entire scene has been loaded.
  //
  virtual void scene_setup (const Scene &scene);

private:

  Ref<Envmap> envmap;

  // Center and radius of a bounding sphere for the engire scene.
  //
  Pos scene_center;
  dist_t scene_radius;
};


}

#endif /* __ENVMAP_LIGHT_H__ */

// arch-tag: a6e2dd05-d36c-487b-9ba4-38c4d2f238b0
