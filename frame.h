// frame.h -- Frame of reference
//
//  Copyright (C) 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __FRAME_H__
#define __FRAME_H__

#include "pos.h"
#include "vec.h"


namespace snogray {


// A frame is a coordinate-system basis, plus a position.
// Positions and vectors may be converted to or from the frame.
//
class Frame
{
public:

  Frame (const Pos &org, const Vec &_x, const Vec &_y, const Vec &_z)
    : origin (org), x (_x), y (_y), z (_z)
    { }
  
  // An "identity" frame.
  //
  Frame () : origin (0,0,0), x (1,0,0), y (0,1,0), z (0,0,1) { }

  // An identity frame at the offset ORG.
  //
  Frame (const Pos &org) : origin (org), x (1,0,0), y (0,1,0), z (0,0,1) { }

  // A frame with the basis _X, _Y, _Z, at the origin.
  //
  Frame (const Vec &_x, const Vec &_y, const Vec &_z)
    : origin (0,0,0), x (_x), y (_y), z (_z)
    { }

  // A frame with the given _Z basis vector; the other basis vectors are
  // chosen automatically (no guarantee is made about them except that
  // they will be orthogonal to _Z).
  //
  Frame (const Vec &_z)
    : origin (0,0,0),
      x (_z.perpendicular ().unit ()),
      y (cross (_z, x).unit ()),
      z (_z)
    { }

  // A frame with the given origin and _Z basis vector; the other basis
  // vectors are chosen automatically (no guarantee is made about them
  // except that they will be orthogonal to _Z).
  //
  Frame (const Pos &org, const Vec &_z)
    : origin (org),
      x (_z.perpendicular ().unit ()),
      y (cross (_z, x).unit ()),
      z (_z)
    { }

  // Return VEC converted to this frame of reference.
  //
  Vec to (const Vec &vec) const { return vec.to_basis (x, y, z); }

  // Return POS converted to this frame of reference, as a vector relative
  // to the origin.
  //
  Vec to (const Pos &pos) const { return to (pos - origin); }

  // Return VEC converted from this frame of reference (to the frame of
  // reference in which the frame's basis vectors are defined).
  //
  Vec from (const Vec &vec) const { return vec.from_basis (x, y, z); }

  // The "position" of the frame; used when converting positions.
  //
  Pos origin;

  // The basis vectors for the frame.
  //
  Vec x, y, z;
};


}

#endif // __FRAME_H__
