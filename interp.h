// interp.h -- Interpolation functions
//
//  Copyright (C) 2008  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __INTERP_H__
#define __INTERP_H__


namespace snogray {


// Linear interplation.
//
template<typename T1, typename T2>
T2 linterp (T1 frac, T2 val1, T2 val2)
{
  return val1 + frac * (val2 - val1);
}

// "s curve" function, for interpolation.
//
template<typename T>
T s (T d)
{
  float d3 = d*d*d, d4 = d3*d, d5 = d4*d;
  return 6*d5 - 15*d4 + 10*d3;
}

// Interplation using an "s curve".
//
template<typename T1, typename T2>
T2 sinterp (T1 frac, T2 val1, T2 val2)
{
  return linterp (s (frac), val1, val2);
}


}

#endif // __INTERP_H__
