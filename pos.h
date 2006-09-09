// pos.h -- Position datatype
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __POS_H__
#define __POS_H__

#include "vec.h"

namespace Snogray {

template<typename T>
class TPos : public Tuple3<T>
{
public:

  using Tuple3<T>::x;
  using Tuple3<T>::y;
  using Tuple3<T>::z;

  TPos (T _x, T _y, T _z) : Tuple3<T> (_x, _y, _z) { }
  TPos () { }

  // Allow easy down-casting for sharing code
  //
  template<typename T2>
  TPos (const Tuple3<T2> &t) : Tuple3<T> (t) { }

  using Tuple3<T>::operator*=;
  using Tuple3<T>::operator/=;

  TPos operator+ (const TVec<T> &v) const
  {
    return TPos(x + v.x, y + v.y, z + v.z);
  }
  Vec operator- (const TPos &p2) const
  {
    return Vec(x - p2.x, y - p2.y, z - p2.z);
  }

  TPos operator* (T scale) const
  {
    return TPos (x * scale, y * scale, z * scale);
  }
  TPos operator/ (T denom) const
  {
    return operator* (1 / denom);
  }

  void operator+= (const TVec<T> &p2)
  {
    x += p2.x; y += p2.y; z += p2.z;
  }
  void operator-= (const TVec<T> &p2)
  {
    x -= p2.x; y -= p2.y; z -= p2.z;
  }

  TPos operator* (const Matrix4<T> &xform) const
  {
    return TPos (Tuple3<T>::operator* (xform));
  }
  const TPos &operator*= (const Matrix4<T> &xform)
  {
    TPos temp = *this * xform;
    *this = temp;
    return *this;
  }

  dist_t dist (const TPos &p2) const
  {
    return (*this - p2).length ();
  }

  TPos midpoint (const TPos &p2) const
  {
    return TPos ((x + p2.x) / 2, (y + p2.y) / 2, (z + p2.z) / 2);
  }
};

template<typename T>
static inline TPos<T>
operator* (T scale, const TPos<T> &pos)
{
  return pos * scale;
}

template<typename T>
static std::ostream&
operator<< (std::ostream &os, const TPos<T> &pos)
{
  os << "pos<" << std::setprecision (5) << lim (pos.x)
     << ", " << std::setprecision (5) << lim (pos.y)
     << ", " << std::setprecision (5) << lim (pos.z)
     << ">";
  return os;
}

typedef TPos<coord_t>  Pos;
typedef TPos<scoord_t> SPos;

}

#endif /* __POS_H__ */

// arch-tag: b1fbd699-066c-42c8-9d21-587c24b92f8d
