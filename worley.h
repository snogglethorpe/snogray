// worley.h -- Worley (Voronoi) noise function
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

#ifndef __WORLEY_H__
#define __WORLEY_H__

#include "pos.h"
#include "vec.h"


namespace snogray {


// A class for generating Worley noise.
//
// It follows the implementation technique used in the book "Texturing
// and Modeling, a Procedural Approach, 3rd edition", and the earlier
// paper "A Cellular Texture Basis Function", by Steven Worley.
//
class Worley
{
public:

  // Return, in the array F, the distances from POS to the MAX_N nearest
  // "feature points" (F should have length at least MAX_N).  If any F_n
  // is not found, its distance is set to zero.
  //
  // The average value of F_i seems to be about (1 + 3^-(1/2^(i-1))):
  //   F_0: 1, F_1: 1.333, F_2: 1.577, F_3: 1.760
  //
  // The maximum value of F_i is less stable, but is usually 2.5 - 3,
  // slowly growing with the value of i (typical maximum values:  F_0:
  // 2.4, F_1:  2.55, F_2: 2.6, F_3: 2.75).  A simple method to keep the
  // result in the range 0-1 is just to divide by 3.
  //
  unsigned eval (const Pos &pos, unsigned max_n, float F[]) const;

private:

  // A simple linear-congruential psueudo-random number generator.  The
  // required properties are that it be very fast, and that it should be
  // seedable (quickly) with a single unsigned integer.
  //
  class RandGen
  {
  public:

    RandGen (unsigned _seed) : seed (_seed) { }

    unsigned gen_unsigned ()
    {
      unsigned rval = seed;
      seed = 1402024253 * seed + 586950981;
      return rval;
    }

    float gen_float ()
    {
      return double (gen_unsigned()) * (1. / ~0U);
    }

  private:

    unsigned seed;
  };

  // Hash function to calculate a RNG seed from integer cube
  // coordinates.
  //
  unsigned hash (int x, int y, int z) const
  {
    return 702395077 * x + 915488749 * y + 2120969693 * z;
  }

  // Distance metric for calculating F_i.
  //
  float distance_metric_sq (Vec delta) const
  {
    return delta.length_squared ();
  }

  // Find the feature points in the cube at coordinates X,Y,Z,
  // calculate their distance from POS, and insert the resulting
  // dinstances in their proper positions in the sorted array F, which
  // is of length MAX_N (any new distances which are greater than the
  // existing value F[MAX_N - 1] are ignored).
  //
  // Also, if a new feature-point distance is written to F[0], the
  // integer hash value of the cube is written to ID (otherwise, ID is
  // left unmodified).
  //
  void add_cube_points (int x, int y, int z, const Pos &pos,
			unsigned max_n, float F[], unsigned &id)
    const;

  // A table used to pick the number of points per cube.
  //
  // It is calculated so that the randomly choosing from this table will
  // result in points that approximate a poisson distribution with a mean
  // density of 2.5.
  //
  static const unsigned poisson_count[256];

  // Point coordinate scaling factor, which results in a mean value of 1.0
  // for F_0.
  //
  static const float DENSITY_ADJUSTMENT;
};


}

#endif // __WORLEY_H__
