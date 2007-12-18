// phog.h -- Phong material
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __PHONG_H__
#define __PHONG_H__

#include "material.h"


namespace snogray {


class Phong : public Material
{
public:

  Phong (const Color &_col, const Color &_spec_col, float _exponent)
    : color (_col), specular_color (_spec_col), exponent (_exponent)
  { }

  // Return a new BRDF object for this material instantiated at ISEC.
  //
  virtual Brdf *get_brdf (const Intersect &isec) const;

  Color color, specular_color;

  float exponent;
};


}

#endif /* __PHONG_H__ */


// arch-tag: 4d54b6d1-a774-4a04-bacc-734927ab6c67
