// lambert.h -- Lambertian material
//
//  Copyright (C) 2005, 2006, 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __LAMBERT_H__
#define __LAMBERT_H__

#include "tex.h"
#include "color.h"

#include "material.h"


namespace snogray {


class Lambert : public Material
{
public:

  Lambert (const TexVal<Color> &col) : color (col) { }

  // Return a new BSDF object for this material instantiated at ISEC.
  //
  virtual Bsdf *get_bsdf (const Intersect &isec) const;

  TexVal<Color> color;
};


}

#endif // __LAMBERT_H__


// arch-tag: ca8981f3-5471-4e8a-ac8b-2e3e54c83b64
