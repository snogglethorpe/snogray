// worley-tex.h -- Worley (Voronoi) noise texture source
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

#ifndef SNOGRAY_WORLEY_TEX_H
#define SNOGRAY_WORLEY_TEX_H

#include "worley.h"

#include "tex.h"


namespace snogray {


// A Worley (Voronoi) noise texture source.
// 
// It takes a series of coefficients C_1 ... C_n as parameters, and
// calculates the final texture value as (C_1 * F_1) + ... + (C_n * F_n);
// (n is the constant MAX_N).
//
class WorleyTex : public Tex<float>
{
public:

  static const unsigned MAX_N = 4;

  WorleyTex (float _coef[MAX_N])
  {
    for (unsigned i = 0; i < MAX_N; i++)
      coef[i] = _coef[i];
  }

  virtual float eval (const TexCoords &coords) const
  {
    float F[MAX_N];

    unsigned id = worley.eval (coords.pos, MAX_N, F);

    float val = 0;
    for (unsigned i = 0; i < MAX_N; i++)
      val += coef[i] * F[i];

    return val;
  }

private:

  Worley worley;

  float coef[MAX_N];
};



// Similar to WorleyTex, but returns a fixed "id" number for each cell,
// which is adjusted to fit a specified range..
//
class WorleyIdTex : public Tex<float>
{
public:

  // Which sort of conversion we use to translate the integer ID to a
  // floating-point output value.
  //
  enum Kind {
    SCALE,		     // scale to fit an output range
    MOD			     // take modulo, keeping as an integer value
  };

  WorleyIdTex (Kind _kind, float max)
    : kind (_kind), bias (0), scale (max)
  { }
  WorleyIdTex (Kind _kind, float min, float max)
    : kind (_kind), bias (min), scale (max - min)
  { }

  virtual float eval (const TexCoords &coords) const
  {
    float F_0;
    unsigned id = worley.eval (coords.pos, 1, &F_0);
    double did = double (id);

    if (kind == MOD)
      did = fmod (did, scale);
    else
      did *= (1. / ~0U) * scale;

    return float (did) + bias;
  }

private:

  Worley worley;

  Kind kind;

  float bias;
  double scale;
};


}

#endif // SNOGRAY_WORLEY_TEX_H
