// frame.h -- Frame of reference
//
//  Copyright (C) 2007, 2008, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __FRAME_H__
#define __FRAME_H__

#include "pos.h"
#include "vec.h"
#include "xform-base.h"


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

  // A frame corresponding to the given transformation matrix.
  //
  Frame (const Matrix4<dist_t> &M)
    : origin (Pos (M (3, 0), M (3, 1), M (3, 2))),
      x (Vec (M (0, 0), M (0, 1), M (0, 2))),
      y (Vec (M (1, 0), M (1, 1), M (1, 2))),
      z (Vec (M (2, 0), M (2, 1), M (2, 2)))
  { }

  // A frame with the given _Z basis vector; the other basis vectors are
  // chosen automatically (no guarantee is made about them except that
  // they will be orthogonal to _Z).
  //
  Frame (const Vec &_z)
    : origin (0,0,0),
      x (_z.perpendicular ().unit ()),
      y (cross (x, _z).unit ()),
      z (_z)
    { }

  // A frame with the given origin and _Z basis vector; the other basis
  // vectors are chosen automatically (no guarantee is made about them
  // except that they will be orthogonal to _Z).
  //
  Frame (const Pos &org, const Vec &_z)
    : origin (org),
      x (_z.perpendicular ().unit ()),
      y (cross (x, _z).unit ()),
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

  // Return POS converted from this frame of reference (to the frame of
  // reference in which the frame's basis vectors are defined).
  //
  Pos from (const Pos &pos) const { return origin + from (Vec (pos)); }

  // The "position" of the frame; used when converting positions.
  //
  Pos origin;

  // The basis vectors for the frame.
  //
  Vec x, y, z;
};


}

#endif // __FRAME_H__
