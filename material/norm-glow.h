// norm-glow.h -- Material whose color indicates surface normal
//
//  Copyright (C) 2007, 2008, 2010-2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_NORM_GLOW_H
#define SNOGRAY_NORM_GLOW_H

#include "material.h"

namespace snogray {

class NormGlow : public Material
{
public:

  NormGlow (intens_t _intens) : Material (EMITS_LIGHT), intens (_intens) { }

  // Return emitted radiance from this light, at the point described by ISEC.
  //
  virtual Color Le (const Intersect &isec) const;

private:

  intens_t intens;
};

}

#endif /* SNOGRAY_NORM_GLOW_H */
