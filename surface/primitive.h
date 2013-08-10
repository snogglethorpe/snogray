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

  // If this surface, or some part of it, uses any light-emitting
  // materials, add appropriate Light objects to LIGHTS.  Any lights
  // added become owned by the owner of LIGHTS, and will be destroyed
  // when it is.
  //
  virtual void add_lights (std::vector<Light *> &lights) const
  {
    material->add_light (*this, lights);
  }

  // Add a single area light, using this surface's shape, to LIGHTS,
  // with with intensity INTENSITY.  An error will be signaled if this
  // surface does not support lighting.
  //
  virtual void add_light (const TexVal<Color> &intensity,
			  std::vector<Light *> &lights)
    const;

  Ref<const Material> material;
};


}


#endif // SNOGRAY_PRIMITIVE_H
