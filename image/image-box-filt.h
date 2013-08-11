// image-box-filt.h -- Boxian filter for image output
//
//  Copyright (C) 2006, 2007, 2010-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_IMAGE_BOX_FILT_H
#define SNOGRAY_IMAGE_BOX_FILT_H

#include "util/snogmath.h"

#include "image-filter.h"


namespace snogray {


class ImageBoxFilt : public ImageFilter
{
public:

  // This should be a simple named constant, but C++ (stupidly)
  // disallows non-integral named constants.  Someday when "constexpr"
  // support is widespread, that can be used instead.
  static float default_radius () { return 0.5; }

  ImageBoxFilt (float _radius = default_radius())
    : ImageFilter (_radius)
  { }
  ImageBoxFilt (const ValTable &params)
    : ImageFilter (params, default_radius())
  { }

  virtual float val (float x_offs, float y_offs) const
  {
    return (abs (x_offs) <= x_radius && abs (y_offs) <= y_radius) ? 1 : 0;
  }
};


}

#endif // SNOGRAY_IMAGE_BOX_FILT_H


// arch-tag: bd67bb8b-e1c1-42db-b623-a4169b5070fc
