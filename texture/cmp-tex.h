// cmp-tex.h -- Texture comparison
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

#ifndef SNOGRAY_CMP_TEX_H
#define SNOGRAY_CMP_TEX_H

#include "tex.h"


namespace snogray {


// A texture which compares two input values, and returns one of two source
// values depending on the result.
//
template<typename T>
class CmpTex : public Tex<T>
{
public:

  enum Op { EQ, NE, LT, LE, GT, GE };

  CmpTex (Op _op,
	  const TexVal<float> &_cval1, const TexVal<float> &_cval2,
	  const TexVal<T> &_rval1, const TexVal<T> &_rval2)
    : op (_op), cval1 (_cval1), cval2 (_cval2), rval1 (_rval1), rval2 (_rval2)
  { }

  // Evaluate this texture at COORDS.
  //
  virtual T eval (const TexCoords &coords) const;

  // The operation.
  //
  Op op;

  // Values to compare.
  //
  TexVal<float> cval1, cval2;

  // Values to return based on comparison result.
  //
  TexVal<T> rval1, rval2;
};


} // namespace snogray


// Include method definitions
//
#include "cmp-tex.tcc"


#endif // SNOGRAY_CMP_TEX_H
