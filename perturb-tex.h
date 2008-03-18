// perturb-tex.h -- Textures for perturbing texture coordinates
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

#ifndef __PERTURB_TEX_H__
#define __PERTURB_TEX_H__

#include "tex.h"


namespace snogray {


template<typename T>
class PerturbPosTex : public Tex<T>
{
public:

  PerturbPosTex (const TexVal<T> &_source,
		 const TexVal<float> &_x, const TexVal<float> &_y,
		 const TexVal<float> &_z)
    : source (_source), x (_x), y (_y), z (_z)
  { }

  virtual T eval (const TexCoords &coords) const
  {
    Vec offs (x.eval (coords), y.eval (coords), z.eval (coords));
    return source.eval (TexCoords (coords.pos + offs, coords.uv));
  }

private:

  TexVal<T> source;

  TexVal<float> x, y, z;
};


template<typename T>
class PerturbUvTex : public Tex<T>
{
public:

  PerturbUvTex (const TexVal<T> &_source,
		const TexVal<float> &_u, const TexVal<float> &_v)
    : source (_source), u (_u), v (_v)
  { }

  virtual T eval (const TexCoords &coords) const
  {
    UV offs (u.eval (coords), v.eval (coords));
    return source.eval (TexCoords (coords.pos, coords.uv + offs));
  }

private:

  TexVal<T> source;

  TexVal<float> u, v;
};


}

#endif // __PERTURB_TEX_H__
