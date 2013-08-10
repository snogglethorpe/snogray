// perlin.h -- Perlin noise function
//
//  Copyright (C) 2008, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_PERLIN_H
#define SNOGRAY_PERLIN_H

#include "geometry/pos.h"
#include "geometry/vec.h"

#include "tex.h"


namespace snogray {


// A class for generating Perlin noise
//
class Perlin
{
public:

  Perlin ();

  // Return Perlin noise at position POS, with a range of -1 to 1.
  //
  float noise (const Pos &pos) const;

private:

  static const unsigned P_LEN = 256;
  static const unsigned G_LEN = 16;

  static int P_global[P_LEN];
  static Vec G_global[G_LEN];

  Vec g (int i, int j, int k) const
  {
    return G[P[(P[(P[i % P_LEN] + j) % P_LEN] + k) % P_LEN] % G_LEN];
  }

  static void init_globals ();

  static bool globals_initialized;

  // Table of permutations.
  //
  int *P;

  // Table of gradients.
  //
  Vec *G;
};


}

#endif // SNOGRAY_PERLIN_H
