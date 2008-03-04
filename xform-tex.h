// xform-tex.h -- Texture coordinate transform
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

#ifndef __XFORM_TEX_H__
#define __XFORM_TEX_H__

#include "tex.h"


namespace snogray {


// A texture which transforms the texture coordinates.
// Both 2d and 3d coordinates are transformed.
//
template<typename T>
class XformTex : public Tex<T>
{
public:

  XformTex (const Xform &_xform, const TexVal<T> &_tex)
    : xform (_xform), tex (_tex)
  { }

  // Evaluate this texture at TEX_COORDS.
  //
  virtual T eval (const TexCoords &tex_coords) const
  {
    Pos xpos = tex_coords.pos * xform;
    UV xuv = tex_coords.uv * xform;
    return tex.eval (TexCoords (xpos, xuv));
  }

  // Transformation to use.  The same transform is used for both 2d and 3d
  // coordinates (the 2d coordinates are mapped to the x-y plane).
  //
  Xform xform;

  // Texture which will be used to texture the transformed coordinates.
  //
  TexVal<T> tex;
};


}

#endif // __XFORM_TEX_H__
