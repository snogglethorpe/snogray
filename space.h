// space.h -- Definitions related to describing 3D space
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SPACE_H__
#define __SPACE_H__

namespace Snogray {

#define Eps	0.000001

// Normal (high precision) coordinates
//
typedef double coord_t;
typedef coord_t dist_t;

// "short" coordinates for use where space is critical
//
typedef float scoord_t;
typedef scoord_t sdist_t;

template<typename T>
inline coord_t lim (T v) { return (v < Eps && v > -Eps) ? 0 : v; }

}

#endif /* __SPACE_H__ */

// arch-tag: 545c5b4f-ae0d-41a1-a743-e285876c5580
