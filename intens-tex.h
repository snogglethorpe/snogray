// intens-tex.h -- color-to-float conversion texture
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

#ifndef SNOGRAY_INTENS_TEX_H
#define SNOGRAY_INTENS_TEX_H

#include "tex.h"
#include "color.h"


namespace snogray {


// A texture which converts Color values to their intensity
//
class IntensTex : public Tex<float>
{
public:

  IntensTex (const TexVal<Color> &_val) : val (_val) { }

  // Evaluate this texture at TEX_COORDS.
  //
  virtual float eval (const TexCoords &tex_coords) const
  {
    return val.eval (tex_coords).intensity ();
  }

  // Color to be converted.
  //
  TexVal<Color> val;
};


}


#endif // SNOGRAY_INTENS_TEX_H
