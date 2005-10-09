// transform.h -- Transformation matrices
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include <cmath>
#include <memory>

#include "space.h"

namespace Snogray {

class Transform
{
public:

  typedef float el_t;

  static Transform translation (dist_t x_offs, dist_t y_offs, dist_t z_offs)
  {
    Transform xform;
    xform (3, 0) = x_offs;
    xform (3, 1) = y_offs;
    xform (3, 2) = z_offs;
    return xform;
  }

  static Transform x_rotation (el_t angle)
  {
    Transform xform;
    el_t sin_a = sin (angle), cos_a = cos (angle);
    xform (1, 1) = cos_a;
    xform (2, 1) = -sin_a;
    xform (1, 2) = sin_a;
    xform (2, 2) = cos_a;
    return xform;
  }

  static Transform y_rotation (el_t angle)
  {
    Transform xform;
    el_t sin_a = sin (angle), cos_a = cos (angle);
    xform (0, 0) = cos_a;
    xform (2, 0) = sin_a;
    xform (0, 2) = -sin_a;
    xform (2, 2) = cos_a;
    return xform;
  }

  static Transform z_rotation (el_t angle)
  {
    Transform xform;
    el_t sin_a = sin (angle), cos_a = cos (angle);
    xform (0, 0) = cos_a;
    xform (1, 0) = -sin_a;
    xform (0, 1) = sin_a;
    xform (1, 1) = cos_a;
    return xform;
  }

  static Transform rotation (el_t x_angle, el_t y_angle, el_t z_angle)
  {
    return x_rotation (x_angle) * y_rotation (y_angle) * z_rotation (z_angle);
  }

  static Transform scaling (el_t s_x, el_t s_y, el_t s_z)
  {
    Transform xform;
    xform (0, 0) = s_x;
    xform (1, 1) = s_y;
    xform (2, 2) = s_z;
    return xform;
  }
  static Transform scaling (el_t scale)
  {
    return scaling (scale, scale, scale);
  }

  // Default constructor returns the identity transformation
  //
  Transform ()
  {
    memset (els, 0, sizeof els);
    for (unsigned i = 0; i < 4; i++)
      els[i][i] = 1;
  }

  el_t &operator() (unsigned i, unsigned j) { return els[i][j]; }
  const el_t &operator() (unsigned i, unsigned j) const { return els[i][j]; }

  Transform operator* (const Transform &xform);

  const Transform &operator*= (const Transform &xform)
  {
    Transform temp = *this * xform;
    *this = temp;
    return *this;
  }

  el_t els[4][4];

};

}

#endif /* __TRANSFORM_H__ */

// arch-tag: e168d3bc-8dfe-4a9a-8708-a79db007005e
