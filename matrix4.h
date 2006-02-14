// matrix4.h -- 4 x 4 matrices
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __MATRIX4_H__
#define __MATRIX4_H__

#include <cmath>

namespace Snogray {

template<typename T>
class Matrix4
{
public:

  static const Matrix4 identity;

  // Default constructor returns an identity matrix
  //
  Matrix4 ()
  {
    els[0][0] = 1; els[0][1] = 0; els[0][2] = 0; els[0][3] = 0;
    els[1][0] = 0; els[1][1] = 1; els[1][2] = 0; els[1][3] = 0;
    els[2][0] = 0; els[2][1] = 0; els[2][2] = 1; els[2][3] = 0;
    els[3][0] = 0; els[3][1] = 0; els[3][2] = 0; els[3][3] = 1;
  }
  Matrix4 (T d0, T d1, T d2, T d3)
  {
    els[0][0] = d0; els[0][1] = 0; els[0][2] = 0; els[0][3] = 0;
    els[1][0] = 0; els[1][1] = d1; els[1][2] = 0; els[1][3] = 0;
    els[2][0] = 0; els[2][1] = 0; els[2][2] = d2; els[2][3] = 0;
    els[3][0] = 0; els[3][1] = 0; els[3][2] = 0; els[3][3] = d3;
  }

  T &operator() (unsigned i, unsigned j) { return els[i][j]; }
  const T &operator() (unsigned i, unsigned j) const { return els[i][j]; }

  T &el (unsigned i, unsigned j) { return els[i][j]; }
  const T &el (unsigned i, unsigned j) const { return els[i][j]; }

  Matrix4 operator* (const Matrix4 &xform) const
  {
    Matrix4<T> result;
    for (unsigned i = 0; i < 4; i++)
      for (unsigned j = 0; j < 4; j++)
	result(i, j)
	  = el (i, 0) * xform (0, j)
	  + el (i, 1) * xform (1, j)
	  + el (i, 2) * xform (2, j)
	  + el (i, 3) * xform (3, j);
    return result;
  }

  Matrix4 operator* (T scale) const
  {
    Matrix4<T> result;
    for (unsigned i = 0; i < 4; i++)
      for (unsigned j = 0; j < 4; j++)
	result(i, j) = el (i, j) * scale;
    return result;
  }

  const Matrix4 &operator*= (const Matrix4 &mat)
  {
    *this = *this * mat;
    return *this;
  }
  const Matrix4 &operator*= (T scale)
  {
    *this = *this * scale;
    return *this;
  }

  Matrix4 &invert ()
  {
    *this = this->inverse ();
    return *this;
  }

  Matrix4 transpose () const
  {
    Matrix4 result;
    result (0, 0) = el (0, 0); result (0, 1) = el (1, 0);
    result (0, 2) = el (2, 0); result (0, 3) = el (3, 0);
    result (1, 0) = el (0, 1); result (1, 1) = el (1, 1);
    result (1, 2) = el (2, 1); result (1, 3) = el (3, 1);
    result (2, 0) = el (0, 2); result (2, 1) = el (1, 2);
    result (2, 2) = el (2, 2); result (2, 3) = el (3, 2);
    result (3, 0) = el (0, 3); result (3, 1) = el (1, 3);
    result (3, 2) = el (2, 3); result (3, 3) = el (3, 3);
    return result;
  }

  T det () const
  {
    return
      el(0,3) * el(1,2) * el(2,1) * el(3,0)
      - el(0,2) * el(1,3) * el(2,1) * el(3,0)
      - el(0,3) * el(1,1) * el(2,2) * el(3,0)
      + el(0,1) * el(1,3) * el(2,2) * el(3,0)
      + el(0,2) * el(1,1) * el(2,3) * el(3,0)
      - el(0,1) * el(1,2) * el(2,3) * el(3,0)
      - el(0,3) * el(1,2) * el(2,0) * el(3,1)
      + el(0,2) * el(1,3) * el(2,0) * el(3,1)
      + el(0,3) * el(1,0) * el(2,2) * el(3,1)
      - el(0,0) * el(1,3) * el(2,2) * el(3,1)
      - el(0,2) * el(1,0) * el(2,3) * el(3,1)
      + el(0,0) * el(1,2) * el(2,3) * el(3,1)
      + el(0,3) * el(1,1) * el(2,0) * el(3,2)
      - el(0,1) * el(1,3) * el(2,0) * el(3,2)
      - el(0,3) * el(1,0) * el(2,1) * el(3,2)
      + el(0,0) * el(1,3) * el(2,1) * el(3,2)
      + el(0,1) * el(1,0) * el(2,3) * el(3,2)
      - el(0,0) * el(1,1) * el(2,3) * el(3,2)
      - el(0,2) * el(1,1) * el(2,0) * el(3,3)
      + el(0,1) * el(1,2) * el(2,0) * el(3,3)
      + el(0,2) * el(1,0) * el(2,1) * el(3,3)
      - el(0,0) * el(1,2) * el(2,1) * el(3,3)
      - el(0,1) * el(1,0) * el(2,2) * el(3,3)
      + el(0,0) * el(1,1) * el(2,2) * el(3,3);
  }

  Matrix4 adjoint () const
  {
    Matrix4<T> result;

    result(0,0) =
      el(1,2)*el(2,3)*el(3,1) - el(1,3)*el(2,2)*el(3,1)
      + el(1,3)*el(2,1)*el(3,2) - el(1,1)*el(2,3)*el(3,2)
      - el(1,2)*el(2,1)*el(3,3) + el(1,1)*el(2,2)*el(3,3);
    result(0,1) =
      el(0,3)*el(2,2)*el(3,1) - el(0,2)*el(2,3)*el(3,1)
      - el(0,3)*el(2,1)*el(3,2) + el(0,1)*el(2,3)*el(3,2)
      + el(0,2)*el(2,1)*el(3,3) - el(0,1)*el(2,2)*el(3,3);
    result(0,2) =
      el(0,2)*el(1,3)*el(3,1) - el(0,3)*el(1,2)*el(3,1)
      + el(0,3)*el(1,1)*el(3,2) - el(0,1)*el(1,3)*el(3,2)
      - el(0,2)*el(1,1)*el(3,3) + el(0,1)*el(1,2)*el(3,3);
    result(0,3) =
      el(0,3)*el(1,2)*el(2,1) - el(0,2)*el(1,3)*el(2,1)
      - el(0,3)*el(1,1)*el(2,2) + el(0,1)*el(1,3)*el(2,2)
      + el(0,2)*el(1,1)*el(2,3) - el(0,1)*el(1,2)*el(2,3);
    result(1,0) =
      el(1,3)*el(2,2)*el(3,0) - el(1,2)*el(2,3)*el(3,0)
      - el(1,3)*el(2,0)*el(3,2) + el(1,0)*el(2,3)*el(3,2)
      + el(1,2)*el(2,0)*el(3,3) - el(1,0)*el(2,2)*el(3,3);
    result(1,1) =
      el(0,2)*el(2,3)*el(3,0) - el(0,3)*el(2,2)*el(3,0)
      + el(0,3)*el(2,0)*el(3,2) - el(0,0)*el(2,3)*el(3,2)
      - el(0,2)*el(2,0)*el(3,3) + el(0,0)*el(2,2)*el(3,3);
    result(1,2) =
      el(0,3)*el(1,2)*el(3,0) - el(0,2)*el(1,3)*el(3,0)
      - el(0,3)*el(1,0)*el(3,2) + el(0,0)*el(1,3)*el(3,2)
      + el(0,2)*el(1,0)*el(3,3) - el(0,0)*el(1,2)*el(3,3);
    result(1,3) =
      el(0,2)*el(1,3)*el(2,0) - el(0,3)*el(1,2)*el(2,0)
      + el(0,3)*el(1,0)*el(2,2) - el(0,0)*el(1,3)*el(2,2)
      - el(0,2)*el(1,0)*el(2,3) + el(0,0)*el(1,2)*el(2,3);
    result(2,0) =
      el(1,1)*el(2,3)*el(3,0) - el(1,3)*el(2,1)*el(3,0)
      + el(1,3)*el(2,0)*el(3,1) - el(1,0)*el(2,3)*el(3,1)
      - el(1,1)*el(2,0)*el(3,3) + el(1,0)*el(2,1)*el(3,3);
    result(2,1) =
      el(0,3)*el(2,1)*el(3,0) - el(0,1)*el(2,3)*el(3,0)
      - el(0,3)*el(2,0)*el(3,1) + el(0,0)*el(2,3)*el(3,1)
      + el(0,1)*el(2,0)*el(3,3) - el(0,0)*el(2,1)*el(3,3);
    result(2,2) =
      el(0,1)*el(1,3)*el(3,0) - el(0,3)*el(1,1)*el(3,0)
      + el(0,3)*el(1,0)*el(3,1) - el(0,0)*el(1,3)*el(3,1)
      - el(0,1)*el(1,0)*el(3,3) + el(0,0)*el(1,1)*el(3,3);
    result(2,3) =
      el(0,3)*el(1,1)*el(2,0) - el(0,1)*el(1,3)*el(2,0)
      - el(0,3)*el(1,0)*el(2,1) + el(0,0)*el(1,3)*el(2,1)
      + el(0,1)*el(1,0)*el(2,3) - el(0,0)*el(1,1)*el(2,3);
    result(3,0) =
      el(1,2)*el(2,1)*el(3,0) - el(1,1)*el(2,2)*el(3,0)
      - el(1,2)*el(2,0)*el(3,1) + el(1,0)*el(2,2)*el(3,1)
      + el(1,1)*el(2,0)*el(3,2) - el(1,0)*el(2,1)*el(3,2);
    result(3,1) =
      el(0,1)*el(2,2)*el(3,0) - el(0,2)*el(2,1)*el(3,0)
      + el(0,2)*el(2,0)*el(3,1) - el(0,0)*el(2,2)*el(3,1)
      - el(0,1)*el(2,0)*el(3,2) + el(0,0)*el(2,1)*el(3,2);
    result(3,2) =
      el(0,2)*el(1,1)*el(3,0) - el(0,1)*el(1,2)*el(3,0)
      - el(0,2)*el(1,0)*el(3,1) + el(0,0)*el(1,2)*el(3,1)
      + el(0,1)*el(1,0)*el(3,2) - el(0,0)*el(1,1)*el(3,2);
    result(3,3) =
      el(0,1)*el(1,2)*el(2,0) - el(0,2)*el(1,1)*el(2,0)
      + el(0,2)*el(1,0)*el(2,1) - el(0,0)*el(1,2)*el(2,1)
      - el(0,1)*el(1,0)*el(2,2) + el(0,0)*el(1,1)*el(2,2);

    return result;
  }

  Matrix4 inverse () const
  {
    return adjoint () * (1 / det ());
  }

private:

  T els[4][4];
};

template<typename T> const Matrix4<T> Matrix4<T>::identity;

}

#endif /* __MATRIX4_H__ */

// arch-tag: f013901a-016f-4c68-b102-c5f4c7a5b4a8
