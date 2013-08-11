// image-filter.h -- Filters for image output
//
//  Copyright (C) 2006, 2007, 2011-2013  Miles Bader <miles@gnu.org>
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

#include "util/snogmath.h"
#include "util/val-table.h"


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

  float x_radius, y_radius;
  float inv_x_radius, inv_y_radius;

protected:

  ImageFilter (float _x_radius, float _y_radius)
    : x_radius (_x_radius), y_radius (_y_radius),
      inv_x_radius (1 / x_radius), inv_y_radius (1 / y_radius)
  { }
  ImageFilter (float _radius)
    : x_radius (_radius), y_radius (_radius),
      inv_x_radius (1 / x_radius), inv_y_radius (1 / y_radius)
  { }

  ImageFilter (const ValTable &params, float def_radius)
    : x_radius (params.get_float ("x_radius,xr,radius,r,x_width,xw,width,w",
				  def_radius)
	       * params.get_float ("x_radius_scale", 1)),
      y_radius (params.get_float ("y_radius,yr,radius,r,y_width,yw,width,w",
				  def_radius)
	       * params.get_float ("y_radius_scale", 1)),
      inv_x_radius (1 / x_radius), inv_y_radius (1 / y_radius)
  { }
};


}

#endif // SNOGRAY_IMAGE_FILTER_H


// arch-tag: 872c9e08-6d72-4d0b-89ca-d5423c1ea696
