// bbox.h -- Axis-aligned bounding boxes
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __BBOX_H__
#define __BBOX_H__

#include "pos.h"
#include "vec.h"
#include "xform-base.h"


namespace snogray {


// An axis-aligned bounding box.
//
class BBox
{
public:

  // An empty bounding box contains reversed bounds, so that any point or
  // bounding box added to it will initialize it to contain exactly that
  // point/bbox.
  //
  BBox ()
    : min (Pos (MAX_COORD, MAX_COORD, MAX_COORD)),
      max (Pos (MIN_COORD, MIN_COORD, MIN_COORD))
  { }

  // Be careful that every component of _MAX is greater than that of _MIN!
  //
  BBox (const Pos &_min, const Pos &_max)
    : min (_min), max (_max)
  { }
  BBox (const Pos &_pos)
    : min (_pos), max (_pos)
  { }

  // Return a bounding box with a given size
  //
  BBox (const Pos &_min, dist_t size)
    : min (_min), max (_min.x + size, _min.y + size, _min.z + size)
  { }

  // Extend this bbox as necessary to enclose POS.
  //
  BBox &
  operator+= (const Pos &pos)
  {
    min = snogray::min (min, pos);
    max = snogray::max (max, pos);
    return *this;
  }

  // Extend this bbox as necessary to enclose BBOX.
  //
  BBox &
  operator+= (const BBox &bbox)
  {
    min = snogray::min (min, bbox.min);
    max = snogray::max (max, bbox.max);
    return *this;
  }

  // Return this bounding-box transformed by XFORM, ensuring that the
  // result is still axis-aligned.
  //
  BBox operator* (const XformBase<dist_t> &xform) const;
  BBox &operator*= (const XformBase<dist_t> &xform)
  {
    *this = *this * xform;
    return *this;
  }

  // Return a vector holding the sizes of this bounding box along all
  // three axes.
  //
  Vec extent () const
  {
    return Vec (max.x - min.x, max.y - min.y, max.z - min.z);
  }

  // The greatest component of its extent
  //
  dist_t max_size () const
  {
    Vec ext = extent ();
    dist_t sz = ext.x;
    if (ext.y > sz)
      sz = ext.y;
    if (ext.z > sz)
      sz = ext.z;
    return sz;
  }

  // The least component of its extent
  //
  dist_t min_size () const
  {
    Vec ext = extent ();
    dist_t sz = ext.x;
    if (ext.y < sz)
      sz = ext.y;
    if (ext.z < sz)
      sz = ext.z;
    return sz;
  }

  // The average dimension
  //
  dist_t avg_size () const
  {
    Vec ext = extent ();
    return (ext.x + ext.y + ext.z) / 3;
  }

  // The median dimension
  //
  dist_t median_size () const
  {
    Vec ext = extent ();
    dist_t min_xy = snogray::min (ext.x, ext.y);
    dist_t max_xy = snogray::max (ext.x, ext.y);
    return min_xy > ext.z ? min_xy : snogray::min (max_xy, ext.z);
  }    

  // Every component of MAX is greater than or equal to the
  // corresponding component of MIN.
  //
  Pos min, max;
};


// Adding two bboxes yields the minimum bbox enclosing both.
//
inline BBox
operator+ (const BBox &bbox1, const BBox &bbox2)
{
  return BBox (min (bbox1.min, bbox2.min), max (bbox1.max, bbox2.max));
}

// Adding a bbox and a point yields the bbox extended to also enclose
// the point.
//
inline BBox
operator+ (const BBox &bbox, const Pos &pos)
{
  return BBox (min (bbox.min, pos), max (bbox.max, pos));
}
inline BBox
operator+ (const Pos &pos, const BBox &bbox)
{
  return BBox (min (bbox.min, pos), max (bbox.max, pos));
}


extern std::ostream& operator<< (std::ostream &os, const BBox &bbox);


}

#endif /* __BBOX_H__ */


// arch-tag: 598693e3-61e0-4b84-b80c-fe37d3c5fea6
