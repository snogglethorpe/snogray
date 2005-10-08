// light.h -- Light object
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "color.h"

namespace Snogray {

class Light {
public:

  Light (const Pos &_pos, const Color &col)
    : pos (_pos), color (col), num (0)
  { }
  Light (const Pos &_pos, float intens = 1, const Color &col = Color::white)
    : pos (_pos), color (col * intens), num (0)
  { }

  Pos pos;
  Color color;

  // Each light has a number, which we use as a index to access various
  // data structures referring to lights.
  //
  unsigned num;
};

}

#endif /* __LIGHT_H__ */

// arch-tag: 07d0a36e-d44f-44f8-bb69-e57c9681de14
