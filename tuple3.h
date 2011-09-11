// tuple3.h -- Base class for positions and vectors
//
//  Copyright (C) 2005, 2006, 2007, 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_TUPLE3_H
#define SNOGRAY_TUPLE3_H

namespace snogray {

template<typename T>
class Tuple3
{
public:

  Tuple3 (T _x, T _y, T _z) : x (_x), y (_y), z (_z) { }
  Tuple3 () : x (0), y (0), z (0) { }

  // Allow easy "tuple-casting"
  //
  template<typename T2>
  Tuple3 (const Tuple3<T2> &t) : x (t.x), y (t.y), z (t.z) { }

  // Mainly so we can use Tuple3<T> as a map key
  //
  bool operator< (const Tuple3<T> &t) const
  {
    return x < t.x || (x == t.x && (y < t.y || (y == t.y && z < t.z)));
  }
  bool operator== (const Tuple3<T> &t) const
  {
    return x == t.x && y == t.y && z == t.z;
  }
  bool operator!= (const Tuple3<T> &t) const
  {
    return ! operator== (t);
  }

  void operator*= (T denom)
  {
    x *= denom; y *= denom; z *= denom;
  }
  void operator/= (T denom)
  {
    operator*= (1 / denom);
  }

  // Indexed access to tuple elements.
  //
  // This uses the hack of treating the address of the "x" field as an
  // array and indexing off of it to access the "y" and "z" fields as
  // well.
  //
  // It would be much prettier to instead put those fields inside an
  // anonymous union along with an array of size three, but that causes
  // gcc to generate significantly worse code (I suppose the union
  // causes it to pessimize alias analysis).
  //
  T &operator[] (unsigned i) { return (&x)[i]; }
  const T &operator[] (unsigned i) const { return (&x)[i]; }

  T x, y, z;
};

}

#endif /* SNOGRAY_TUPLE3_H */

// arch-tag: c796c67c-4af6-4fec-8f17-47bf6128c047
