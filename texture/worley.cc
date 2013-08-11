// worley.cc -- Worley (Voronoi) noise function
//
//  Copyright (C) 2008, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "util/snogmath.h"

#include "worley.h"


using namespace snogray;


// A table used to pick the number of points per cube.
//
// It is calculated so that the randomly choosing from this table will
// result in points that approximate a poisson distribution with a mean
// density of 2.5.
//
// This data is from the book "Texturing and Modeling, a Procedural
// Approach, 3rd edition"
//
const unsigned
Worley::poisson_count[256] = {
  4,3,1,1,1,2,4,2,2,2,5,1,0,2,1,2,2,0,4,3,2,1,2,1,3,2,2,4,2,2,5,1,2,3,
  2,2,2,2,2,3,2,4,2,5,3,2,2,2,5,3,3,5,2,1,3,3,4,4,2,3,0,4,2,2,2,1,3,2,
  2,2,3,3,3,1,2,0,2,1,1,2,2,2,2,5,3,2,3,2,3,2,2,1,0,2,1,1,2,1,2,2,1,3,
  4,2,2,2,5,4,2,4,2,2,5,4,3,2,2,5,4,3,3,3,5,2,2,2,2,2,3,1,1,4,2,1,3,3,
  4,3,2,4,3,3,3,4,5,1,4,2,4,3,1,2,3,5,3,2,1,3,1,3,3,3,2,3,1,5,5,4,2,2,
  4,1,3,4,1,5,3,3,5,3,4,3,2,2,1,1,1,1,1,2,4,5,4,5,4,2,1,5,1,1,2,3,3,3,
  2,5,2,3,3,2,0,2,1,1,4,2,1,3,2,1,2,2,3,2,5,5,3,4,5,5,2,4,4,5,3,2,2,2,
  1,4,2,3,3,4,2,5,4,2,4,2,2,2,4,5,3,2
};

// Point coordinate scaling factor, which results in a mean value of 1.0
// for F_0.
//
const float Worley::DENSITY_ADJUSTMENT = 0.398150;


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
unsigned
Worley::eval (const Pos &pos, unsigned max_n, float F[]) const
{
  const float MAX_DIST = 9999;	// greater than any possible real result

  Pos adj_pos = pos * DENSITY_ADJUSTMENT;

  // Initialize F[] with values that will be greater than any real
  // value.
  //
  for (unsigned i = 0; i < max_n; i++)
    F[i] = MAX_DIST;

  // Find the integer coordinates of the cube ADJ_POS is in.
  //
  int x = int (floor (adj_pos.x));
  int y = int (floor (adj_pos.y));
  int z = int (floor (adj_pos.z));

  // Position of the cube.
  //
  Pos cube_pos (x, y, z);

  //
  // Note that all distance calculations in the body of the algorithm
  // use the _square_ of the real distances, to avoid square-roots.  We
  // take the square-root of the final result just before returning.
  //

  // An arbitrary integer "id" (hash value) for the cube containing F[0].
  //
  unsigned id = 0;

  // Process feature points in this cube.
  //
  add_cube_points (x,y,z, adj_pos, max_n, F, id);

  // Calculate maximum distances (squared) from ADJ_POS to neighoring
  // rows of cubes in either direction.  We'll use those to quickly
  // reject neighboring cubes -- if the distance to the neighbor cube is
  // greater than the current maximum result, no point inside will
  // affect the results, so we can skip that neighbor cube entirely.
  //
  Vec lower = adj_pos - cube_pos;
  Vec upper = Vec(1,1,1) - lower;

  // we're using distances squared for calcs, so calculate those.
  //
  float l2x = lower.x * lower.x;
  float l2y = lower.y * lower.y;
  float l2z = lower.z * lower.z;
  float u2x = upper.x * upper.x;
  float u2y = upper.y * upper.y;
  float u2z = upper.z * upper.z;

  // "Face" neighbor cubes.  Because these are slighty closer than edge
  // and corner neighbor cubes, we do them first, as that increases the
  // chance of quick rejection for lather neighbors.
  //
  if (l2x < F[max_n - 1])
    add_cube_points (x-1, y, z, adj_pos, max_n, F, id);
  if (l2y < F[max_n - 1])
    add_cube_points (x, y-1, z, adj_pos, max_n, F, id);
  if (l2z < F[max_n - 1])
    add_cube_points (x, y, z-1, adj_pos, max_n, F, id);
  
  if (u2x < F[max_n - 1])
    add_cube_points (x+1, y, z, adj_pos, max_n, F, id);
  if (u2y < F[max_n - 1])
    add_cube_points (x, y+1, z, adj_pos, max_n, F, id);
  if (u2z < F[max_n - 1])
    add_cube_points (x, y, z+1, adj_pos, max_n, F, id);
  
  // Next, "edge" neighbor cubes.
  //
  if (l2x + l2y < F[max_n - 1])
    add_cube_points (x-1, y-1, z, adj_pos, max_n, F, id);
  if (l2x + l2z < F[max_n - 1])
    add_cube_points (x-1, y, z-1, adj_pos, max_n, F, id);
  if (l2y + l2z < F[max_n - 1])
    add_cube_points (x, y-1, z-1, adj_pos, max_n, F, id);  
  if (u2x + u2y < F[max_n - 1])
    add_cube_points (x+1, y+1, z, adj_pos, max_n, F, id);
  if (u2x + u2z < F[max_n - 1])
    add_cube_points (x+1, y, z+1, adj_pos, max_n, F, id);
  if (u2y + u2z < F[max_n - 1])
    add_cube_points (x, y+1, z+1, adj_pos, max_n, F, id);  
  if (l2x + u2y < F[max_n - 1])
    add_cube_points (x-1, y+1, z, adj_pos, max_n, F, id);
  if (l2x + u2z < F[max_n - 1])
    add_cube_points (x-1, y, z+1, adj_pos, max_n, F, id);
  if (l2y + u2z < F[max_n - 1])
    add_cube_points (x, y-1, z+1, adj_pos, max_n, F, id);  
  if (u2x + l2y < F[max_n - 1])
    add_cube_points (x+1, y-1, z, adj_pos, max_n, F, id);
  if (u2x + l2z < F[max_n - 1])
    add_cube_points (x+1, y, z-1, adj_pos, max_n, F, id);
  if (u2y + l2z < F[max_n - 1])
    add_cube_points (x, y+1, z-1, adj_pos, max_n, F, id);  
  
  // Finally, "corner" neighbor cubes.
  //
  if (l2x + l2y + l2z < F[max_n - 1])
    add_cube_points (x-1, y-1, z-1, adj_pos, max_n, F, id);
  if (l2x + l2y + u2z < F[max_n - 1])
    add_cube_points (x-1, y-1, z+1, adj_pos, max_n, F, id);
  if (l2x + u2y + l2z < F[max_n - 1])
    add_cube_points (x-1, y+1, z-1, adj_pos, max_n, F, id);
  if (l2x + u2y + u2z < F[max_n - 1])
    add_cube_points (x-1, y+1, z+1, adj_pos, max_n, F, id);
  if (u2x + l2y + l2z < F[max_n - 1])
    add_cube_points (x+1, y-1, z-1, adj_pos, max_n, F, id);
  if (u2x + l2y + u2z < F[max_n - 1])
    add_cube_points (x+1, y-1, z+1, adj_pos, max_n, F, id);
  if (u2x + u2y + l2z < F[max_n - 1])
    add_cube_points (x+1, y+1, z-1, adj_pos, max_n, F, id);
  if (u2x + u2y + u2z < F[max_n - 1])
    add_cube_points (x+1, y+1, z+1, adj_pos, max_n, F, id);

  // Take the square-root of the results (since we've been using
  // distance-squared measures until now), and re-scale the result to
  // reverse our initial coordinate scaling.
  //
  float inv_adj = 1 / DENSITY_ADJUSTMENT;
  for (unsigned i = 0; i  <  max_n; i++)
    F[i] = (F[i] == MAX_DIST) ? 0 : sqrt (F[i]) * inv_adj;

  return id;
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
void
Worley::add_cube_points (int x, int y, int z, const Pos &pos,
			 unsigned max_n, float F[], unsigned &id)
  const
{
  unsigned hv = hash (x, y, z);
  RandGen rand (hv);

  unsigned cube_id = rand.gen_unsigned ();

  unsigned m = poisson_count[(cube_id >> 24) & 0xFF];

  while (m > 0)
    {
      m--;

      float fx = x + rand.gen_float ();
      float fy = y + rand.gen_float ();
      float fz = z + rand.gen_float ();

      Pos fpoint (fx, fy, fz);

      float dist = distance_metric_sq (fpoint - pos);

      // If DIST is less than the current maximum distance, insert it in
      // sorted order into F.
      //
      if (dist < F[max_n - 1])
	{
	  unsigned i;

	  for (i = max_n - 1; i > 0 && dist < F[i - 1]; --i)
	    F[i] = F[i - 1];

	  F[i] = dist;

	  if (i == 0)
	    id = cube_id;
	}
    }
}
