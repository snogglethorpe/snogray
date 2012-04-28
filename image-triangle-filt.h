// triangle.h -- Triangleian filter for image output
//
//  Copyright (C) 2010-2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_IMAGE_TRIANGLE_FILT_H
#define SNOGRAY_IMAGE_TRIANGLE_FILT_H

#include "image-filter.h"


namespace snogray {


class ImageTriangleFilt : public ImageFilter
{
public:

  // This should be a simple named constant, but C++ (stupidly)
  // disallows non-integral named constants.  Someday when "constexpr"
  // support is widespread, that can be used instead.
  static float default_radius () { return 2; }

  ImageTriangleFilt (float _radius = default_radius())
    : ImageFilter (_radius)
  { }
  ImageTriangleFilt (const ValTable &params)
    : ImageFilter (params, default_radius())
  { }

  virtual float val (float x, float y) const
  {
    return max (0.f, x_radius - abs (x)) * max (0.f, y_radius - abs (y));
  }
};


}

#endif // SNOGRAY_IMAGE_TRIANGLE_FILT_H
