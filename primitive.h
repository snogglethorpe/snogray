// primitive.h -- Physical primitive
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

#ifndef __PRIMITIVE_H__
#define __PRIMITIVE_H__

#include "surface.h"
#include "material.h"


namespace snogray {


// A primitive is a subclass of Surface that also stores a material.  It
// is the superclass of most concrete objects.
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

  Ref<const Material> material;
};


}


#endif // __PRIMITIVE_H__
