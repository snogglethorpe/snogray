// gauss.h -- Gaussian filter for image output
//
//  Copyright (C) 2006, 2007, 2010-2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_IMAGE_GAUSS_FILT_H
#define SNOGRAY_IMAGE_GAUSS_FILT_H

#include "gaussian-filter.h"

#include "image-filter.h"


namespace snogray {


// 2D gauss
//
class ImageGaussFilt : public ImageFilter
{
public:

  static float default_alpha () { return 8; }
  static float default_radius () { return 2; }

  ImageGaussFilt (float _alpha = default_alpha(),
		  float _radius = default_radius())
    : ImageFilter (_radius),
      alpha (_alpha),
      x_filter (x_radius, _alpha),
      y_filter (y_radius, _alpha)
  { }
  ImageGaussFilt (const ValTable &params)
    : ImageFilter (params, default_radius()),
      alpha (params.get_float ("alpha,a", default_alpha())),
      x_filter (x_radius, alpha),
      y_filter (y_radius, alpha)
  { }

  virtual float val (float x, float y) const
  {
    return x_filter (x) * y_filter (y);
  }

  float alpha;

private:

  // Separable filters for x- and y-dimensions.
  //
  GaussianFilter<float> x_filter, y_filter;
};


}

#endif // SNOGRAY_IMAGE_GAUSS_FILT_H


// arch-tag: 6599c368-7f8e-4636-8ff8-6eb1688f033a
