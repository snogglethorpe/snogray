// xform-tex.h -- Texture coordinate transform
//
//  Copyright (C) 2008, 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_XFORM_TEX_H
#define SNOGRAY_XFORM_TEX_H


#include "geometry/xform.h"

#include "tex.h"


namespace snogray {


// Base class for XformTex classes
//
template<typename T>
class XformTexBase : public Tex<T>
{
public:

  XformTexBase (const Xform &_xform, const TexVal<T> &_tex)
    : xform (_xform), tex (_tex)
  { }

  // Transformation to use.  The same transform is used for both 2d and 3d
  // coordinates (the 2d coordinates are mapped to the x-y plane).
  //
  Xform xform;

  // Texture which will be used to texture the transformed coordinates.
  //
  TexVal<T> tex;

protected:

  // If TEX refers to _another_ XformTexBase subclass of type T, then
  // merge its transform into XFORM, and point TEX at its input.
  //
  template<class XT>
  void merge_input_transform ()
  {
    const XT *input_xform_tex = dynamic_cast<const XT *> (&*tex.tex);
    if (input_xform_tex)
      {
	xform *= input_xform_tex->xform;
	tex = input_xform_tex->tex;
      }
  }
};


// A texture which transforms the texture coordinates.
// Both UV and positional coordinates are transformed.
//
template<typename T>
class XformTex : public XformTexBase<T>
{
public:

  XformTex (const Xform &_xform, const TexVal<T> &_tex)
    : XformTexBase<T> (_xform, _tex)
  {
    this->template merge_input_transform<XformTex<T> > ();
  }

  // Evaluate this texture at TEX_COORDS.
  //
  virtual T eval (const TexCoords &tex_coords) const
  {
    Pos xpos = XformTexBase<T>::xform (tex_coords.pos);
    UV xuv = XformTexBase<T>::xform (tex_coords.uv);
    return XformTexBase<T>::tex.eval (TexCoords (xpos, xuv));
  }
};


// A texture which transforms texture UV coordinates.
//
template<typename T>
class XformTexUV : public XformTexBase<T>
{
public:

  XformTexUV (const Xform &_xform, const TexVal<T> &_tex)
    : XformTexBase<T> (_xform, _tex)
  {
    this->template merge_input_transform<XformTexUV<T> > ();
  }

  // Evaluate this texture at TEX_COORDS.
  //
  virtual T eval (const TexCoords &tex_coords) const
  {
    UV xuv = XformTexBase<T>::xform (tex_coords.uv);
    return XformTexBase<T>::tex.eval (TexCoords (tex_coords.pos, xuv));
  }
};


// A texture which transforms texture positional coordinates.
//
template<typename T>
class XformTexPos : public XformTexBase<T>
{
public:

  XformTexPos (const Xform &_xform, const TexVal<T> &_tex)
    : XformTexBase<T> (_xform, _tex)
  {
    this->template merge_input_transform<XformTexPos<T> > ();
  }

  // Evaluate this texture at TEX_COORDS.
  //
  virtual T eval (const TexCoords &tex_coords) const
  {
    Pos xpos = XformTexBase<T>::xform (tex_coords.pos);
    return XformTexBase<T>::tex.eval (TexCoords (xpos, tex_coords.uv));
  }
};


}

#endif // SNOGRAY_XFORM_TEX_H
