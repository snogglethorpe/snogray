// misc-tex.h -- Miscellaneous texture sources
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

#ifndef __MISC_TEX_H__
#define __MISC_TEX_H__

#include "snogmath.h"

#include "tex.h"


namespace snogray {


// A sine-wave texture.
//
template<typename T>
class SinTex : public Tex<T>
{
public:
  virtual T eval (const TexCoords &tex_coords) const
  {
    return sin (tex_coords.uv.u * PI * 2) * 0.5f + 0.5f;
  }
};


// A triangle-wave texture.
//
template<typename T>
class TriTex : public Tex<T>
{
public:
  virtual T eval (const TexCoords &tex_coords) const
  {
    return abs (fmod (tex_coords.uv.u, 1) * 2 - 1);
  }
};


}

#endif // __MISC_TEX_H__
