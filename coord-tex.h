// coord-tex.h -- Texture access to raw texture coordinates
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

#ifndef __COORD_TEX_H__
#define __COORD_TEX_H__

#include "tex.h"


namespace snogray {


class CoordTex : public Tex<float>
{
public:

  // Types of coordinates that can be returned.
  //
  enum Kind { X, Y, Z, U, V };

  CoordTex (Kind _kind) : kind (_kind) { }

  virtual float eval (const TexCoords &coords) const
  {
    switch (kind)
      {
      case X: return coords.pos.x;
      case Y: return coords.pos.y;
      case Z: return coords.pos.z;
      case U: return coords.uv.u;
      case V: return coords.uv.v;
      default: return 0;
      }
  }

private:

  Kind kind;
};


}

#endif // __COORD_TEX_H__
