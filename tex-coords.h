// tex-coords.h -- Structure to hold texture coordinates
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

#ifndef SNOGRAY_TEX_COORDS_H
#define SNOGRAY_TEX_COORDS_H

#include "uv.h"
#include "pos.h"


namespace snogray {


class TexCoords
{
public:

  TexCoords (const Pos &_pos, const UV &_uv)
    : pos (_pos), uv (_uv)
  { }

  Pos pos;
  UV uv;
};


}

#endif // SNOGRAY_TEX_COORDS_H
