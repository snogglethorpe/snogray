// image-filter.h -- Filters for image output
//
//  Copyright (C) 2006, 2007, 2011, 2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_IMAGE_FILTER_H
#define SNOGRAY_IMAGE_FILTER_H

#include "snogmath.h"
#include "val-table.h"


namespace snogray {


// 2D filter
//
class ImageFilter
{
public:

  // Return a new a filter depending on the parameters in PARAMS.
  //
  static ImageFilter *make (const ValTable &params);

  virtual ~ImageFilter () { }

  virtual float val (float x, float y) const = 0;

  float operator() (float x, float y) const { return val (x, y); }

  float x_width, y_width;
  float inv_x_width, inv_y_width;

protected:

  ImageFilter (float _x_width, float _y_width)
    : x_width (_x_width), y_width (_y_width),
      inv_x_width (1 / x_width), inv_y_width (1 / y_width)
  { }
  ImageFilter (float _width)
    : x_width (_width), y_width (_width),
      inv_x_width (1 / x_width), inv_y_width (1 / y_width)
  { }

  ImageFilter (const ValTable &params, float def_width)
    : x_width (params.get_float ("x_width,xw,width,w", def_width)
	       * params.get_float ("x_width_scale", 1)),
      y_width (params.get_float ("y_width,yw,width,w", def_width)
	       * params.get_float ("y_width_scale", 1)),
      inv_x_width (1 / x_width), inv_y_width (1 / y_width)
  { }
};


}

#endif // SNOGRAY_IMAGE_FILTER_H


// arch-tag: 872c9e08-6d72-4d0b-89ca-d5423c1ea696
