// cmp-tex.tcc -- Texture comparison
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

#ifndef __CMP_TEX_TCC__
#define __CMP_TEX_TCC__

#include "config.h"

#include "color.h"


namespace snogray {


// Evaluate this texture at COORDS.
//
template<typename T>
T
CmpTex<T>::eval (const TexCoords &coords) const
{
  T c1 = cval1.eval (coords);
  T c2 = cval2.eval (coords);

  bool cmp_result = false;
  switch (op)
    {
    case EQ:
      cmp_result = (c1 == c2); break;
    case NE:
      cmp_result = (c1 != c2); break;
    case LT:
      cmp_result = (c1 <  c2); break;
    case LE:
      cmp_result = (c1 <= c2); break;
    case GT:
      cmp_result = (c1 >  c2); break;
    case GE:
      cmp_result = (c1 >= c2); break;
    }

  return cmp_result ? rval1.eval (coords) : rval2.eval (coords);
}


// If possible, suppress instantiation of classes which we will define
// out-of-line.
//
// These declarations should be synchronized with the "template class"
// declarations at the end of "cmp-tex.cc".
// 
#if HAVE_EXTERN_TEMPLATE
EXTERN_TEMPLATE_EXTENSION extern template class CmpTex<Color>;
EXTERN_TEMPLATE_EXTENSION extern template class CmpTex<float>;
#endif


} // namespace snogray

#endif // __CMP_TEX_TCC__
