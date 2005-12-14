// transform3.h -- Transformation matrices
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TRANSFORM3_H__
#define __TRANSFORM3_H__

#include <cmath>
#include <memory>

#include "space.h"

namespace Snogray {

template<typename T>
class Transform3
{
public:

  static Transform3 translation (dist_t x_offs, dist_t y_offs, dist_t z_offs)
  {
    Transform3 xform;
    xform (3, 0) = x_offs;
    xform (3, 1) = y_offs;
    xform (3, 2) = z_offs;
    return xform;
  }

  static Transform3 x_rotation (T angle)
  {
    Transform3 xform;
    T sin_a = sin (angle), cos_a = cos (angle);
    xform (1, 1) = cos_a;
    xform (2, 1) = -sin_a;
    xform (1, 2) = sin_a;
    xform (2, 2) = cos_a;
    return xform;
  }

  static Transform3 y_rotation (T angle)
  {
    Transform3 xform;
    T sin_a = sin (angle), cos_a = cos (angle);
    xform (0, 0) = cos_a;
    xform (2, 0) = sin_a;
    xform (0, 2) = -sin_a;
    xform (2, 2) = cos_a;
    return xform;
  }

  static Transform3 z_rotation (T angle)
  {
    Transform3 xform;
    T sin_a = sin (angle), cos_a = cos (angle);
    xform (0, 0) = cos_a;
    xform (1, 0) = -sin_a;
    xform (0, 1) = sin_a;
    xform (1, 1) = cos_a;
    return xform;
  }

  static Transform3 rotation (T x_angle, T y_angle, T z_angle)
  {
    return x_rotation (x_angle) * y_rotation (y_angle) * z_rotation (z_angle);
  }

  static Transform3 scaling (T s_x, T s_y, T s_z)
  {
    Transform3 xform;
    xform (0, 0) = s_x;
    xform (1, 1) = s_y;
    xform (2, 2) = s_z;
    return xform;
  }
  static Transform3 scaling (T scale)
  {
    return scaling (scale, scale, scale);
  }

  // Default constructor returns the identity transform3ation
  //
  Transform3 ()
  {
    memset (els, 0, sizeof els);
    for (unsigned i = 0; i < 4; i++)
      els[i][i] = 1;
  }

  T &operator() (unsigned i, unsigned j) { return els[i][j]; }
  const T &operator() (unsigned i, unsigned j) const { return els[i][j]; }

  Transform3 operator* (const Transform3 &xform);

  const Transform3 &operator*= (const Transform3 &xform)
  {
    Transform3 temp = *this * xform;
    *this = temp;
    return *this;
  }

  T els[4][4];
};

}

#endif /* __TRANSFORM3_H__ */

// arch-tag: e168d3bc-8dfe-4a9a-8708-a79db007005e
