// perlin.h -- Perlin noise function
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

#include <algorithm>

#include "interp.h"

#include "perlin.h"


using namespace snogray;


Perlin::Perlin ()
  : P (P_global), G (G_global)
{
  if (! globals_initialized)
    init_globals ();
}


// Return Perlin noise at position POS, with a range of -1 to 1.
//
float
Perlin::noise (const Pos &pos) const
{
  Pos base (floor (pos.x), floor (pos.y), floor (pos.z));
  Vec frac = pos - base;
  int xi = int (base.x), yi = int (base.y), zi = int (base.z);

  float v000 = dot (g (xi+0, yi+0, zi+0), Vec (0,0,0) - frac);
  float v001 = dot (g (xi+0, yi+0, zi+1), Vec (0,0,1) - frac);
  float v010 = dot (g (xi+0, yi+1, zi+0), Vec (0,1,0) - frac);
  float v011 = dot (g (xi+0, yi+1, zi+1), Vec (0,1,1) - frac);
  float v100 = dot (g (xi+1, yi+0, zi+0), Vec (1,0,0) - frac);
  float v101 = dot (g (xi+1, yi+0, zi+1), Vec (1,0,1) - frac);
  float v110 = dot (g (xi+1, yi+1, zi+0), Vec (1,1,0) - frac);
  float v111 = dot (g (xi+1, yi+1, zi+1), Vec (1,1,1) - frac);

  float v00 = sinterp (frac.z, v000, v001);
  float v01 = sinterp (frac.z, v010, v011);
  float v10 = sinterp (frac.z, v100, v101);
  float v11 = sinterp (frac.z, v110, v111);

  float v0 = sinterp (frac.y, v00, v01);
  float v1 = sinterp (frac.y, v10, v11);

  return sinterp (frac.x, v0, v1);
}


// Global table initialization

bool Perlin::globals_initialized = false;

int Perlin::P_global[P_LEN];

Vec Perlin::G_global[G_LEN];


void
Perlin::init_globals ()
{
  for (unsigned i = 0; i < P_LEN; i++)
    P_global[i] = i;

  std::random_shuffle (&P_global[0], &P_global[P_LEN]);

  G_global[0] = Vec (1,1,0);
  G_global[1] = Vec (-1,1,0);
  G_global[2] = Vec (1,-1,0);
  G_global[3] = Vec (-1,-1,0);
  G_global[4] = Vec (1,0,1);
  G_global[5] = Vec (-1,0,1);
  G_global[6] = Vec (1,0,-1);
  G_global[7] = Vec (-1,0,-1);
  G_global[8] = Vec (0,1,1);
  G_global[9] = Vec (0,-1,1);
  G_global[10] = Vec (0,1,-1);
  G_global[11] = Vec (0,-1,-1);
  G_global[12] = Vec (1,1,0);
  G_global[13] = Vec (-1,1,0);
  G_global[14] = Vec (0,-1,1);
  G_global[15] = Vec (0,-1,-1);
}
