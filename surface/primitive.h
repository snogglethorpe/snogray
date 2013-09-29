// primitive.h -- Primitive surface
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

#ifndef SNOGRAY_PRIMITIVE_H
#define SNOGRAY_PRIMITIVE_H

#include "surface.h"
#include "material/material.h"


namespace snogray {


// Primitive is a subclass of Surface that sort of represents
// "concrete", non-composite, surfaces.  In particular, a Primitive
// object has a material associated with it.
//
// This is an abstract class.
//
class Primitive : public Surface
{
public:

  Primitive (const Ref<const Material> &mat) : material (mat) { }
  virtual ~Primitive () { }

  // Add light-samplers for this surface in SCENE to SAMPLERS.  Any
  // samplers added become owned by the owner of SAMPLERS, and will be
  // destroyed when it is.
  //
  virtual void add_light_samplers (
		 const Scene &scene,
		 std::vector<const Light::Sampler *> &samplers)
    const;

  // Add light-samplers for this primitive's shape, with with intensity
  // INTENSITY, to SAMPLERS.  An error will be signaled if this
  // primitive does not support lighting.
  //
  virtual void add_light_samplers (
		 const TexVal<Color> &intensity,
		 std::vector<const Light::Sampler *> &samplers)
    const;

  // Add statistics about this surface to STATS (see the definition of
  // Surface::Stats below for details).  CACHE is used internally for
  // coordination amongst nested surfaces.
  //
  // This method is intended for internal use in the Surface class
  // hierachy, but cannot be protected: due to pecularities in the way
  // that is defined in C++.
  //
  virtual void accum_stats (Stats &stats, StatsCache &cache) const;

  Ref<const Material> material;
};


}


#endif // SNOGRAY_PRIMITIVE_H
