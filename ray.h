// ray.h -- Datatype describing a directional, positioned, line-segment
//
//  Copyright (C) 2005, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __RAY_H__
#define __RAY_H__

#include <fstream>

#include "pos.h"
#include "vec.h"

namespace snogray {

// A ray is a vector with a position and a length; we include various other
// fields for handy test.
class Ray
{
public:

  Ray (Pos _origin, Vec _extent)
    : origin (_origin), dir (_extent.unit ()), len (_extent.length ()),
      _end (_origin + _extent)
  {
  }
  Ray (Pos _origin, Vec _dir, dist_t _len)
    : origin (_origin), dir (_dir), len (_len),
      _end (_origin + _dir * _len)
  {
  }
  Ray (Pos _origin, Pos _targ)
    : origin (_origin), dir ((_targ - _origin).unit ()),
      len ((_targ - _origin).length ()), _end (_targ)
  {
  }
  Ray (const Ray &ray)
    : origin (ray.origin), dir (ray.dir), len (ray.len), _end (ray._end)
  {
  }
  Ray (const Ray &ray, dist_t length)
    : origin (ray.origin), dir (ray.dir), len (length),
      _end (ray.origin + ray.dir * length)
  {
  }

  /* Returns an end point of the ray izzf it is extended to length LEN.  */
  Pos extension (float _len) const { return origin + dir * _len; }

  Pos end () const { return _end; }

  void set_len (dist_t _len) { len = _len; _end = origin + dir*len; }

  Pos origin;

  Vec dir;			// should always be a unit vector
  dist_t len;

private:

  // This is a pre-computed copy of (ORIGIN + DIR * LEN)
  Pos _end;
};

extern std::ostream& operator<< (std::ostream &os, const Ray &ray);

}

#endif /* __RAY_H__ */

// arch-tag: e8ba773e-11bd-4fb2-83b6-ace5f2908aad
