// light.h -- Light object
//
//  Copyright (C) 2005-2008, 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_LIGHT_H
#define SNOGRAY_LIGHT_H

#include <vector>

#include "color/color.h"
#include "geometry/pos.h"
#include "geometry/vec.h"
#include "geometry/uv.h"
#include "geometry/xform.h"


namespace snogray {

class Intersect;
class Scene;


class Light
{
public:

  class Sampler;

  Light () { }
  virtual ~Light () { }

  // Add light-samplers for this light in SCENE to SAMPLERS.  Any
  // samplers added become owned by the owner of SAMPLERS, and will be
  // destroyed when it is.
  //
  virtual void add_light_samplers (const Scene &scene,
				   std::vector<const Sampler *> &samplers)
    const = 0;

  // Transform the geometry of this light by XFORM.
  //
  virtual void transform (const Xform &xform) = 0;
};


}

#endif /* SNOGRAY_LIGHT_H */

// arch-tag: 07d0a36e-d44f-44f8-bb69-e57c9681de14
