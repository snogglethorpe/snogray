// interp.h -- Interpolation functions
//
//  Copyright (C) 2008, 2011, 2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_INTERP_H
#define SNOGRAY_INTERP_H


namespace snogray {


// Linear interplation.
//
template<typename T1, typename T2>
T2 linterp (T1 frac, T2 val1, T2 val2)
{
  return val1 + T2 (frac) * (val2 - val1);
}

// "s curve" function, for interpolation.
//
template<typename T>
T s (T d)
{
  T d3 = d*d*d, d4 = d3*d, d5 = d4*d;
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

#endif // SNOGRAY_INTERP_H
