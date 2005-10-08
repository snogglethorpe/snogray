// space.h -- Definitions related to describing 3D space
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SPACE_H__
#define __SPACE_H__

namespace Snogray {

typedef float coord_t;
typedef coord_t dist_t;

class Tuple3 {
public:
  Tuple3 (coord_t _x = 0, coord_t _y = 0, coord_t _z = 0)
    : x (_x), y (_y), z (_z)
  { }

  coord_t x, y, z;
};  

}

#endif /* __SPACE_H__ */

// arch-tag: 545c5b4f-ae0d-41a1-a743-e285876c5580
