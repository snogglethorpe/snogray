// norm-glow.h -- Material whose color indicates surface normal
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __NORM_GLOW_H__
#define __NORM_GLOW_H__

#include "material.h"

namespace snogray {

class NormGlow : public Material
{
public:

  NormGlow (Color::component_t _intens)
    : Material (Material::SHADOW_OPAQUE), intens (_intens)
  { }

  virtual Color render (const Intersect &isec) const;

private:

  Color::component_t intens;
};

}

#endif /* __NORM_GLOW_H__ */
