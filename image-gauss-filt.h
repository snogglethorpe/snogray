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

#include "image-filter.h"


namespace snogray {


// 2D gauss
//
class ImageGaussFilt : public ImageFilter
{
public:

  static float default_alpha () { return 2; }
  static float default_radius () { return 2; }

  ImageGaussFilt (float _alpha = default_alpha(),
		  float _radius = default_radius())
    : ImageFilter (_radius),
      alpha (_alpha),
      bias (gauss1 (default_radius (), 0)),
      x_scale (default_radius () / _radius),
      y_scale (default_radius () / _radius)
  { }
  ImageGaussFilt (const ValTable &params)
    : ImageFilter (params, default_radius()),
      alpha (params.get_float ("alpha,a", default_alpha())),
      bias (gauss1 (default_radius (), 0)),
      x_scale (default_radius () / x_radius),
      y_scale (default_radius () / y_radius)
  { }

  virtual float val (float x, float y) const
  {
    return gauss1 (x * x_scale, bias) * gauss1 (y * y_scale, bias);
  }

  float gauss1 (float offs, float _bias) const
  {
    return max (exp (-alpha * offs * offs) - _bias, 0.f);
  }

  float alpha;

private:

  // The value of the gaussian curve at the edge of our coverage.
  // We want the end result to be zero at that point, so we subtract
  // this value from the value calculated.
  //
  float bias;

  // We scale the curve to match our radius, in both x- and
  // y-dimensions, so that the filter coverage and the value of alpha
  // are relatively independent.
  //
  float x_scale, y_scale;
};


}

#endif // SNOGRAY_IMAGE_GAUSS_FILT_H


// arch-tag: 6599c368-7f8e-4636-8ff8-6eb1688f033a
