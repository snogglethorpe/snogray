// glow.h -- Constant-color reflectance function
//
//  Copyright (C) 2005, 2006, 2007, 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __GLOW_H__
#define __GLOW_H__

#include "material.h"


namespace snogray {


class Glow : public Material
{
public:

  Glow (const Color &_color)
    : Material (Material::SHADOW_NONE, true), color (_color)
  { }

  // Return emitted radiance from this light, at the point described by ISEC.
  //
  virtual Color le (const Intersect &isec) const;

private:

  Color color;
};


}

#endif /* __GLOW_H__ */


// arch-tag: d53c41c0-1970-4b3e-9047-2f67dd943922
