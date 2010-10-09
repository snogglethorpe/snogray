// arith-tex.tcc -- arithmetic on textured values
//
//  Copyright (C) 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __ARITH_TEX_TCC__
#define __ARITH_TEX_TCC__

#include "config.h"

#include "color.h"
#include "color-math.h"


namespace snogray {


// Evaluate this texture at TEX_COORDS.
//
template<typename T>
T
ArithTex<T>::eval (const TexCoords &tex_coords) const
{
  T val1 = arg1.eval (tex_coords);
  T val2 = arg2.eval (tex_coords);

  switch (op)
    {
    case ADD:
      return val1 + val2;
    case SUB:
      return val1 - val2;
    case MUL:
      return val1 * val2;
    case DIV:
      return val2 == 0 ? 0 : val1 / val2;
    case MOD:
      return val2 == 0 ? 0 : fmod (val1, val2);
    case POW:
      return pow (val1, val2);
    case FLOOR:
      return floor (val1 / val2) * val2;
    case CEIL:
      return ceil (val1 / val2) * val2;
    case TRUNC:
      return trunc (val1 / val2) * val2;
    case MIN:
      return min (val1, val2);
    case MAX:
      return max (val1, val2);
    case AVG:
      return (val1 + val2) / 2;
    case MIRROR:
      return abs (val1 - val2);
    case SIN:
      return sin (val1 * 2 * PI / val2);
    case COS:
      return cos (val1 * 2 * PI / val2);
    case TAN:
      return tan (val1 * 2 * PI / val2);
    case ATAN2:
      return atan2 (val1, val2);
    default:
      return val1;		// no meaning, just cheap
    };
}


// If possible, suppress instantiation of classes which we will define
// out-of-line.
//
// These declarations should be synchronized with the "template class"
// declarations at the end of "arith-tex.cc".
// 
#if HAVE_EXTERN_TEMPLATE
EXTERN_TEMPLATE_EXTENSION extern template class ArithTex<Color>;
EXTERN_TEMPLATE_EXTENSION extern template class ArithTex<float>;
#endif


} // namespace snogray

#endif // __ARITH_TEX_TCC__
