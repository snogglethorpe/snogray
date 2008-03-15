// misc-map-tex.h -- Miscellaneous coordinate mappings textures
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

#ifndef __MISC_MAP_TEX_H__
#define __MISC_MAP_TEX_H__

#include "snogmath.h"

#include "tex.h"


namespace snogray {


// Texture for mapping from 3d x-y plane to 2d texture coordinates.
//
template<typename T>
class PlaneMapTex : public Tex<T>
{
public:

  PlaneMapTex (const Ref<Tex<T> > &_tex) : tex (_tex) { }

  virtual T eval (const TexCoords &coords) const
  {
    return tex->eval (TexCoords (coords.pos, UV (coords.pos.x, coords.pos.y)));
  }

  const Ref<Tex<T> > tex;
};


// Texture for mapping from a 3d cylinder to 2d texture coordinates.
//
template<typename T>
class CylinderMapTex : public Tex<T>
{
public:

  CylinderMapTex (const Ref<Tex<T> > &_tex) : tex (_tex) { }

  virtual T eval (const TexCoords &coords) const
  {
    const Pos &pos = coords.pos;
    UV uv (atan2 (pos.x, pos.y) * INV_PI * 0.5f + 0.5f, pos.z);
    return tex->eval (TexCoords (pos, uv));
  }

  const Ref<Tex<T> > tex;
};


// Texture for mapping from a 3d sphere to 2d texture coordinates, using
// a "latitude-longitude" mapping.
//
template<typename T>
class LatLongMapTex : public Tex<T>
{
public:

  LatLongMapTex (const Ref<Tex<T> > &_tex) : tex (_tex) { }

  virtual T eval (const TexCoords &coords) const
  {
    const Pos &pos = coords.pos;
    float x = pos.x, y = pos.y, z = pos.z;
    UV uv (atan2 (x, y) * INV_PI * 0.5f + 0.5f,
	   atan2 (z, sqrt (x*x + y*y)) * INV_PI + 0.5f);
    return tex->eval (TexCoords (pos, uv));
  }

  const Ref<Tex<T> > tex;
};


}

#endif // __MISC_MAP_TEX_H__
