// filter.h -- Filter datatype
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __FILTER_H__
#define __FILTER_H__

#include <cmath>
#include <algorithm> 		// for std::max

#include "params.h"

namespace Snogray {

// 2D filter
//
class Filter
{
public:

  virtual ~Filter ();

  float max_width () const
  {
    return std::max (x_width, y_width);
  }

  virtual float val (float x, float y) const = 0;

  float operator() (float x, float y) const { return val (x, y); }

  float x_width, y_width;
  float inv_x_width, inv_y_width;

protected:

  Filter (float _x_width, float _y_width)
    : x_width (_x_width), y_width (_y_width),
      inv_x_width (1 / x_width), inv_y_width (1 / y_width)
  { }

  Filter (const Params &params, float def_width)
    : x_width (params.get_float ("x-width", def_width)),
      y_width (params.get_float ("y-width", def_width)),
      inv_x_width (1 / x_width), inv_y_width (1 / y_width)
  { }
};

}

#endif /* __FILTER_H__ */

// arch-tag: 872c9e08-6d72-4d0b-89ca-d5423c1ea696
