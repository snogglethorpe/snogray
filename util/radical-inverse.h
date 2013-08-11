// radical-inverse.h -- Compute "radical inverse" of a number
//
//  Copyright (C) 2010, 2011  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_RADICAL_INVERSE_H
#define SNOGRAY_RADICAL_INVERSE_H


namespace snogray {


// Compute the radical inverse of NUM in base BASE.
//
// If the integer NUM is expressed in base BASE as the sequence of
// digits D_i D_i-1 ... D_0, then the radical inverse of NUM is a
// fractional number with the same digits mirrored about the point:
// 0 . D_0 D_1 ... D_i.
//
static inline double
radical_inverse (unsigned num, unsigned base)
{
  double val = 0;
  double inv_base = 1 / double (base);
  double inv_bi = inv_base;

  while (num > 0)
    {
      unsigned d_i = num % base;
      val += d_i * inv_bi;
      num /= base;
      inv_bi *= inv_base;
    }

  return val;
}


}

#endif // SNOGRAY_RADICAL_INVERSE_H
