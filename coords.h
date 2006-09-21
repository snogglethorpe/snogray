// coords.h -- Definitions related to describing 3D coords
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __COORDS_H__
#define __COORDS_H__

namespace Snogray {

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
typedef dcoord_t coord_t;
typedef ddist_t dist_t;

template<typename T>
inline T lim (T v) { return (v < Eps && v > -Eps) ? 0 : v; }

}

#endif /* __COORDS_H__ */

// arch-tag: 545c5b4f-ae0d-41a1-a743-e285876c5580
