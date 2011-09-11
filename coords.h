// coords.h -- Definitions related to describing 3D coords
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

#ifndef SNOGRAY_COORDS_H
#define SNOGRAY_COORDS_H

#include <limits>

#include "config.h"


namespace snogray {

// This value should be comfortably smaller than the smallest possible
// surface.
//
#define Eps	(1e-15)

// single-precision coordinates for use where space is critical
//
typedef float scoord_t;
typedef scoord_t sdist_t;

// double-precision coordinates, for where precision is critical
//
typedef double dcoord_t;
typedef dcoord_t ddist_t;

// Normal coordinates
//
#if USE_FLOAT_COORDS
  typedef scoord_t coord_t;
  typedef sdist_t dist_t;
#else
  typedef dcoord_t coord_t;
  typedef ddist_t dist_t;
#endif  

// We'll probably never hit these limits, so just use the smaller float
// values for both scoord_t and dcoord_t.
//
// Note that we define MIN_COORD as -MAX_COORD; we can't use
// "std::numeric_limits<float>::min()" because that's actually the minimum
// _positive_ value of a float.
//
#define MAX_COORD std::numeric_limits<float>::max()
#define MIN_COORD (-MAX_COORD)

}

#endif /* SNOGRAY_COORDS_H */

// arch-tag: 545c5b4f-ae0d-41a1-a743-e285876c5580
