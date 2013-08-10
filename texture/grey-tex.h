// grey-tex.h -- float-to-color conversion texture
//
//  Copyright (C) 2008, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_GREY_TEX_H
#define SNOGRAY_GREY_TEX_H

#include "tex.h"
#include "color.h"


namespace snogray {


// A texture which converts floating-point values to (grey) color
//
class GreyTex : public Tex<Color>
{
public:

  GreyTex (const TexVal<float> &_val) : val (_val) { }

  // Evaluate this texture at TEX_COORDS.
  //
  virtual Color eval (const TexCoords &tex_coords) const
  {
    return val.eval (tex_coords);
  }

  TexVal<float> val;
};


}


#endif // SNOGRAY_GREY_TEX_H
