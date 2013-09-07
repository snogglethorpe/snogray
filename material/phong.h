// phog.h -- Phong material
//
//  Copyright (C) 2005-2007, 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_PHONG_H
#define SNOGRAY_PHONG_H

#include "material.h"


namespace snogray {


class Phong : public Material
{
public:

  Phong (const Color &_col, const Color &_spec_col, float _exponent)
    : color (_col), specular_color (_spec_col), exponent (_exponent)
  { }

  // Return a new BSDF object for this material instantiated at ISEC.
  //
  virtual Bsdf *get_bsdf (const Intersect &isec, const TexCoords &tex_coords)
    const;

  Color color, specular_color;

  float exponent;
};


}

#endif /* SNOGRAY_PHONG_H */


// arch-tag: 4d54b6d1-a774-4a04-bacc-734927ab6c67
