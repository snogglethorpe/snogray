// bbox.h -- Axis-aligned bounding boxes
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
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

  // Grows the bbox as necessary to include POS
  //
  void include (const Pos &pos)
  {
    // Note that minimum/maximum adjusting cases may not be mutually
    // exclusive in the case of a degenerate (uninitialized) bounding box,
    // so we need to check both.

    if (pos.x < min.x)
      min.x = pos.x;
    if (pos.x > max.x)
      max.x = pos.x;

    if (pos.y < min.y)
      min.y = pos.y;
    if (pos.y > max.y)
      max.y = pos.y;

    if (pos.z < min.z)
      min.z = pos.z;
    if (pos.z > max.z)
      max.z = pos.z;
  }

  // Return a vector holding the sizes of this bounding box along all three
  // axes.
  
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

inline std::ostream&
operator<< (std::ostream &os, const BBox &bbox)
{
  os << "bbox<"
     << std::setprecision (5) << lim (bbox.min.x) << ", "
     << std::setprecision (5) << lim (bbox.min.y) << ", "
     << std::setprecision (5) << lim (bbox.min.z) << " - "
     << std::setprecision (5) << lim (bbox.max.x) << ", "
     << std::setprecision (5) << lim (bbox.max.y) << ", "
     << std::setprecision (5) << lim (bbox.max.z)
     << ">";
  return os;
}


}

#endif /* __BBOX_H__ */


// arch-tag: 598693e3-61e0-4b84-b80c-fe37d3c5fea6
