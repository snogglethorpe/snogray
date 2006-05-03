// box.h -- Boxian filter
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __BOX_H__
#define __BOX_H__

#include "filter.h"

namespace Snogray {

class Box : public Filter
{
public:

  static const float DEFAULT_WIDTH = 0.5;

  Box (float _x_width = DEFAULT_WIDTH, float _y_width = DEFAULT_WIDTH)
    : Filter (_x_width, _y_width)
  { }
  Box (const Params &params)
    : Filter (params, DEFAULT_WIDTH)
  { }
  ~Box ();

  virtual float val (float x, float y) const;
};

}

#endif /* __BOX_H__ */

// arch-tag: bd67bb8b-e1c1-42db-b623-a4169b5070fc
