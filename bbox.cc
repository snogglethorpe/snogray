// bbox.cc -- Axis-aligned bounding boxes
//
//  Copyright (C) 2007, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "xform.h"

#include "bbox.h"


using namespace snogray;


// Transform this bounding-box by XFORM, ensuring that the result is
// still axis-aligned.
//
BBox
BBox::operator* (const XformBase<dist_t> &xform) const
{
  BBox xformed_bbox (xform (min));

  xformed_bbox += xform (Pos (min.x, min.y, max.z));
  xformed_bbox += xform (Pos (min.x, max.y, min.z));
  xformed_bbox += xform (Pos (min.x, max.y, max.z));
  xformed_bbox += xform (Pos (max.x, min.y, min.z));
  xformed_bbox += xform (Pos (max.x, min.y, max.z));
  xformed_bbox += xform (Pos (max.x, max.y, min.z));

  xformed_bbox += xform (max);

  return xformed_bbox;
}


// arch-tag: fa0045f7-1646-422c-86ec-6375b51ae950
