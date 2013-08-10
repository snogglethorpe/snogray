// envmap.h -- Environment maps
//
//  Copyright (C) 2006-2008, 2011, 2012, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_ENVMAP_H
#define SNOGRAY_ENVMAP_H

#include "ref.h"
#include "color.h"
#include "geometry/vec.h"
#include "image.h"


namespace snogray {


class Envmap : public RefCounted
{
public:

  virtual ~Envmap () { }

  // Return the color of this environment map in direction DIR.
  //
  virtual Color map (const Vec &dir) const = 0;

  // Return a "light-map" -- a lat-long format spheremap image
  // containing light values of the environment map -- for this
  // environment map.
  //
  virtual Ref<Image> light_map () const = 0;
};


}

#endif // SNOGRAY_ENVMAP_H
