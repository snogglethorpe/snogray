// material.h -- Surface material datatype
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "color.h"
#include "ray.h"
#include "ref.h"
#include "tex.h"


namespace snogray {

class Light;
class Intersect;
class Medium;
class Brdf;


class Material : public RefCounted
{
public:

  Material (bool emits_light = false)
    : bump_map (0), _emits_light (emits_light)
  { }
  virtual ~Material () { }

  // Return a new BRDF object for this material instantiated at ISEC.
  //
  virtual Brdf *get_brdf (const Intersect &/*isec*/) const { return 0; }

  // Return the medium of this material (used only for refraction).
  //
  virtual const Medium *medium () const { return 0; }

  // Return emitted radiance from this light, at the point described by ISEC.
  //
  virtual Color le (const Intersect &/*isec*/) const { return 0; }

  // Return true if this material emits light.
  //
  bool emits_light () const { return _emits_light; }

  Ref<const Tex<float> > bump_map;

private:

  // Cache this info for speed...
  //
  bool _emits_light;
};


}

#endif /* __MATERIAL_H__ */


// arch-tag: 4e4442a2-254d-4635-bcf5-a03508c2057e
