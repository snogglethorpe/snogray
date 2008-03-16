// perlin-tex.h -- Perlin noise texture source
//
//  Copyright (C) 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __PERLIN_TEX_H__
#define __PERLIN_TEX_H__

#include "perlin.h"

#include "tex.h"


namespace snogray {


class PerlinTex : public Tex<float>
{
public:

  virtual float eval (const TexCoords &coords) const
  {
    return perlin.noise (coords.pos);
  }

private:

  Perlin perlin;
};


}

#endif // __PERLIN_TEX_H__
