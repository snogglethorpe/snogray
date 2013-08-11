// interp-tex.h -- Interpolation textures
//
//  Copyright (C) 2008, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_INTERP_TEX_H
#define SNOGRAY_INTERP_TEX_H

#include "util/interp.h"

#include "tex.h"


namespace snogray {


// A linear interpolation texture.
//
template<typename T>
class LinterpTex : public Tex<T>
{
public:

  LinterpTex (const TexVal<float> &_control,
	      const TexVal<T> &_val1, const TexVal<T> &_val2)
    : control (_control), val1 (_val1), val2 (_val2)
  { }

  virtual T eval (const TexCoords &tex_coords) const
  {
    float c = control.eval (tex_coords);
    T v1 = val1.eval (tex_coords);
    T v2 = val2.eval (tex_coords);
    return linterp (c, v1, v2);
  }

private:

  const TexVal<float> control;
  const TexVal<T> val1, val2;
};


// An "s curve" interpolation texture.
//
template<typename T>
class SinterpTex : public Tex<T>
{
public:

  SinterpTex (const TexVal<float> &_control,
	      const TexVal<T> &_val1, const TexVal<T> &_val2)
    : control (_control), val1 (_val1), val2 (_val2)
  { }

  virtual T eval (const TexCoords &tex_coords) const
  {
    float c = control.eval (tex_coords);
    T v1 = val1.eval (tex_coords);
    T v2 = val2.eval (tex_coords);
    return sinterp (c, v1, v2);
  }

private:

  const TexVal<float> control;
  const TexVal<T> val1, val2;
};


}

#endif // SNOGRAY_INTERP_TEX_H
