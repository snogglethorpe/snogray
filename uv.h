// uv.h -- UV datatype, for holding 2d texture coordinates
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __UV_H__
#define __UV_H__


namespace snogray {


// Pair of values
//
template<typename T>
class TUV
{
public:

  TUV (T _u, T _v) : u (_u), v (_v) { }

  T u, v;
};

typedef TUV<float> UV;


}

#endif // __UV_H__


// arch-tag: b1f1e25c-0a80-4db0-a0e4-a4fcf52bfa6e
