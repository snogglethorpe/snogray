// frame.h -- Frame of reference
//
//  Copyright (C) 2007, 2008, 2010-2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_FRAME_H
#define SNOGRAY_FRAME_H

#include "pos.h"
#include "vec.h"
#include "xform-base.h"


namespace snogray {


// A "frame of reference", relative to the world frame, consisting of
// a coordinate-system basis plus an origin.  Positions and vectors
// may be converted to or from the frame (from or to the world-frame,
// respectively).  The origin is only used when converting positions.
//
// A frame is basically equivalent to a transformation matrix in
// functionality, but in many cases is a nicer abstraction.
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

  // A frame corresponding to the transformation matrix FRAME_TO_WORLD.
  //
  Frame (const Matrix4<dist_t> &frame_to_world)
    : origin (
	 frame_to_world (3, 0), frame_to_world (3, 1), frame_to_world (3, 2)),
      x (frame_to_world (0, 0), frame_to_world (0, 1), frame_to_world (0, 2)),
      y (frame_to_world (1, 0), frame_to_world (1, 1), frame_to_world (1, 2)),
      z (frame_to_world (2, 0), frame_to_world (2, 1), frame_to_world (2, 2))
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

  // Return this frame transformed by XFORM.
  //
  Frame transformed (const XformBase<dist_t> &xform) const
  {
    return Frame (xform (origin), xform (x), xform (y), xform (z));
  }

  // Transform this frame by XFORM.
  //
  void transform (const XformBase<dist_t> &xform)
  {
    origin.transform (xform);
    x.transform (xform);
    y.transform (xform);
    z.transform (xform);
  }

  // Return true if this frame reverses handedness.
  //
  bool reverses_handedness () const
  {
    return dot (cross (x, y), z) < 0;
  }

  // The "position" of the frame; used when converting positions.
  //
  Pos origin;

  // The basis vectors for the frame.
  //
  Vec x, y, z;
};


}

#endif // SNOGRAY_FRAME_H
