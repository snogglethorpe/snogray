// tex.h -- texture base class
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

#ifndef __TEX_H__
#define __TEX_H__

#include "ref.h"
#include "intersect.h"
#include "tex-coords.h"


namespace snogray {


typedef float tparam_t;


// A texture.
//
template<typename T>
class Tex : public RefCounted
{
public:

  // Evaluate this texture at TEX_COORDS.
  //
  virtual T eval (const TexCoords &tex_coords) const = 0;
};


// A textured value.  It is either a constant value or refers to a
// texture which can be used to generate a value.
//
template<typename T>
class TexVal
{
public:

  TexVal (const Ref<const Tex<T> > &_tex) : tex (_tex), default_val (0) { }
  TexVal (const T &val) : default_val (val) { }

  TexVal &operator= (const Ref<const Tex<T> > &_tex)
  {
    tex = _tex;
    return *this;
  }
  TexVal &operator= (const T &val)
  {
    tex = 0;
    default_val = val;
    return *this;
  }

  // Evaluate this texture at ISEC..
  //
  T eval (const Intersect &isec) const
  {
    return eval (isec.tex_coords);
  }

  // Evaluate this texture at TEX_COORDS.
  //
  T eval (const TexCoords &tex_coords) const
  {
    return tex ? tex->eval (tex_coords) : default_val;
  }

  Ref<const Tex<T> > tex;

  T default_val;
};


}

#endif // __TEX_H__
