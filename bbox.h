// bbox.h -- Bounding boxes
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __BBOX_H__
#define __BBOX_H__

#include "pos.h"
#include "vec.h"

namespace Snogray {

class BBox
{
public:
  // Be careful that every component of _MAX is greater than that of _MIN!
  BBox (const Pos &_min, const Pos &_max)
    : min (_min), max (_max)
  { }
  BBox (const Pos &_pos)
    : min (_pos), max (_pos)
  { }
  // Returns a bounding box with a given size
  BBox (const Pos &_min, dist_t size)
    : min (_min), max (_min.x + size, _min.y + size, _min.z + size)
  { }

  // Grows the bbox as necessary to include POS
  void include (const Pos &pos)
  {
    if (pos.x < min.x)
      min.x = pos.x;
    else if (pos.x > max.x)
      max.x = pos.x;
    if (pos.y < min.y)
      min.y = pos.y;
    else if (pos.y > max.y)
      max.y = pos.y;
    if (pos.z < min.z)
      min.z = pos.z;
    else if (pos.z > max.z)
      max.z = pos.z;
  }

  Vec extent () const
  {
    return Vec (max.x - min.x, max.y - min.y, max.z - min.z);
  }

  // The greatest component of its extent
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
  dist_t avg_size () const
  {
    Vec ext = extent ();
    return (ext.x + ext.y + ext.z) / 3;
  }

  // Every component of MAX is greater than or equal to the
  // corresponding component of MIN.
  Pos min, max;
};

}

#endif /* __BBOX_H__ */

// arch-tag: 598693e3-61e0-4b84-b80c-fe37d3c5fea6
