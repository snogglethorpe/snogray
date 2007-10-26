// box.h -- Boxian filter
//
//  Copyright (C) 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
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
