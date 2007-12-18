// box.h -- Boxian filter
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __BOX_FILT_H__
#define __BOX_FILT_H__

#include "filter.h"


namespace snogray {


class BoxFilt : public Filter
{
public:

  static const float DEFAULT_WIDTH = 0.5;

  BoxFilt (float _width = DEFAULT_WIDTH) : Filter (_width) { }
  BoxFilt (const ValTable &params) : Filter (params, DEFAULT_WIDTH) { }

  virtual float val (float, float) const
  {
    return 1;
  }
};


}

#endif /* __BOX_FILT_H__ */


// arch-tag: bd67bb8b-e1c1-42db-b623-a4169b5070fc
