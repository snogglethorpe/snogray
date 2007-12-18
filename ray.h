// ray.h -- Datatype describing a directional, positioned, line-segment
//
//  Copyright (C) 2005, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __RAY_H__
#define __RAY_H__

#include <fstream>

#include "pos.h"
#include "vec.h"
#include "xform-base.h"


namespace snogray {


// A ray is a vector with a position and a length; we include various other
// fields for handy test.
//
template<typename T>
class TRay
{
public:

  TRay (TPos<T> _origin, TVec<T> _extent)
    : origin (_origin), dir (_extent.unit ()), t0 (0), t1 (_extent.length ())
  {
  }
  TRay (TPos<T> _origin, TVec<T> _dir, T _t1)
    : origin (_origin), dir (_dir), t0 (0), t1 (_t1)
  {
  }
  TRay (TPos<T> _origin, TVec<T> _dir, T _t0, T _t1)
    : origin (_origin), dir (_dir), t0 (_t0), t1 (_t1)
  {
  }
  TRay (TPos<T> _origin, TPos<T> _targ)
    : origin (_origin), dir ((_targ - _origin).unit ()),
      t0 (0), t1 ((_targ - _origin).length ())
  {
  }
  TRay (const TRay &ray)
    : origin (ray.origin), dir (ray.dir), t0 (ray.t0), t1 (ray.t1)
  {
  }
  TRay (const TRay &ray, T _t1)
    : origin (ray.origin), dir (ray.dir), t0 (ray.t0), t1 (_t1)
  {
  }
  TRay (const TRay &ray, T _t0, T _t1)
    : origin (ray.origin), dir (ray.dir), t0 (_t0), t1 (_t1)
  {
  }

  /* Returns an end point of the ray if it was extended to length LEN.  */
  TPos<T> extension (float _len) const { return origin + dir * _len; }

  TPos<T> begin () const { return origin+dir*t0; }
  TPos<T> end () const { return  origin+dir*t1; }

  T length () const { return t1 - t0; }

  // Ray transformation.
  //
  TRay operator* (const XformBase<T> &xform) const
  {
    return TRay (origin * xform, dir * xform, t0, t1);
  }

  // The ray starts at ORIGIN, and points in the direction DIR.
  //
  TPos<T> origin;
  TVec<T> dir;

  // The "extent" of the ray:  from ORIGIN+T0*DIR to ORIGIN+T1*DIR.
  //
  T t0, t1;
};


typedef TRay<dist_t> Ray;

extern std::ostream& operator<< (std::ostream &os, const Ray &ray);


}


#endif /* __RAY_H__ */

// arch-tag: e8ba773e-11bd-4fb2-83b6-ace5f2908aad
