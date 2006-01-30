// xform.h -- Transformation matrices
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __XFORM_H__
#define __XFORM_H__

#include "matrix4.h"
#include "coords.h"
#include "pos.h"
#include "vec.h"

namespace Snogray {

template<typename T>
class TXform : public Matrix4<T>
{
public:

  // Default is identity transformation
  //
  TXform (T scale = 1) : Matrix4<T> (scale) { }

  // A vector/position yields a translation
  //
  TXform (const Pos &pos)
  {
    el (3, 0) = pos.x;
    el (3, 1) = pos.y;
    el (3, 2) = pos.z;
  }
  TXform (const Vec &vec)
  {
    el (3, 0) = vec.x;
    el (3, 1) = vec.y;
    el (3, 2) = vec.z;
  }

  // Allow easy down-casting from a raw matrix
  //
  template<typename T2>
  TXform (const Matrix4<T2> &m) : Matrix4<T> (m) { }

  static TXform translation (const TVec<T> &offs)
  {
    return TXform (offs);
  }
  static TXform translation (dist_t x, dist_t y, dist_t z)
  {
    return TXform (TVec<T> (x, y, z));
  }

  static TXform scaling (T scale)
  {
    return TXform (scale);
  }
  static TXform scaling (T s_x, T s_y, T s_z)
  {
    TXform xform;
    xform (0, 0) = s_x;
    xform (1, 1) = s_y;
    xform (2, 2) = s_z;
    return xform;
  }

  static TXform x_rotation (T angle)
  {
    TXform xform;
    T sin_a = sin (angle), cos_a = cos (angle);
    xform (1, 1) = cos_a;
    xform (2, 1) = -sin_a;
    xform (1, 2) = sin_a;
    xform (2, 2) = cos_a;
    return xform;
  }

  static TXform y_rotation (T angle)
  {
    TXform xform;
    T sin_a = sin (angle), cos_a = cos (angle);
    xform (0, 0) = cos_a;
    xform (2, 0) = sin_a;
    xform (0, 2) = -sin_a;
    xform (2, 2) = cos_a;
    return xform;
  }

  static TXform z_rotation (T angle)
  {
    TXform xform;
    T sin_a = sin (angle), cos_a = cos (angle);
    xform (0, 0) = cos_a;
    xform (1, 0) = -sin_a;
    xform (0, 1) = sin_a;
    xform (1, 1) = cos_a;
    return xform;
  }

  static TXform rotation (TVec<T> axis, T angle)
  {
    TXform xform;

    T dx = axis.x, dy = axis.y, dz = axis.z;

    T roll  = -atan2 (dx, sqrt (dy * dy + dz * dz));
    T pitch =  atan2 (dz, dy);

    xform.rotate_x (-pitch);
    xform.rotate_z (-roll);

    xform.rotate_y (angle);

    xform.rotate_z (roll);
    xform.rotate_x (pitch);

    return xform;
  }

  TXform &translate (dist_t x, dist_t y, dist_t z)
  {
    *this *= translation (x, y, z);
    return *this;
  }
  TXform &translate (const TVec<T> &offs)
  {
    *this *= translation (offs);
    return *this;
  }

  TXform &scale (T scale)
  {
    *this *= scale;
    return *this;
  }
  TXform &scale (T s_x, T s_y, T s_z)
  {
    *this *= scaling (s_x, s_y, s_z);
    return *this;
  }

  TXform &rotate_x (T angle)
  {
    *this *= x_rotation (angle);
    return *this;
  }
  TXform &rotate_y (T angle)
  {
    *this *= y_rotation (angle);
    return *this;
  }
  TXform &rotate_z (T angle)
  {
    *this *= z_rotation (angle);
    return *this;
  }
  TXform &rotate (TVec<T> axis, T angle)
  {
    *this *= rotation (axis, angle);
    return *this;
  }

private:

  using Matrix4<T>::el;
};

typedef TXform<dist_t>  Xform;
typedef TXform<sdist_t> SXform;

}

#endif /* __XFORM_H__ */

// arch-tag: e168d3bc-8dfe-4a9a-8708-a79db007005e
