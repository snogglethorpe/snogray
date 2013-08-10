// rescale-tex.h -- Value rescaling texture
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

#ifndef SNOGRAY_RESCALE_TEX_H
#define SNOGRAY_RESCALE_TEX_H

#include "snogmath.h"

#include "tex.h"


namespace snogray {


// A texture which changes the scaling of its input.
//
template<typename T>
class RescaleTex : public Tex<T>
{
public:

  RescaleTex (const TexVal<T> &_val, T in_min, T in_max, T out_min, T out_max)
    : val (_val), in_bias (in_min), out_bias (out_min),
      scale ((out_max - out_min) / (in_max - in_min))
  { }

  virtual T eval (const TexCoords &coords) const
  {
    return (val.eval (coords) - in_bias) * scale + out_bias;
  }

  const TexVal<T> val;

  T in_bias, out_bias, scale;
};


}

#endif // SNOGRAY_RESCALE_TEX_H
