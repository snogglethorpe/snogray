// spherical-coords.h -- Mappings from spherical coordinates to/from vectors
//
//  Copyright (C) 2010-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_SPHERICAL_COORDS_H
#define SNOGRAY_SPHERICAL_COORDS_H

//
// This file contains functions for converting between spherical
// coordinates and vectors.  There are many such mappings, so the
// following definitions are somewhat arbitrarily chosen.
//
// All vector coordinates assume a left-handed coordinate system.
//
// z-axis coordinates:  defined on a sphere with its axis along the z-axis
//
//   latitude:   angle between the vector and the x-y plane, with a
//		 range -PI/2 - PI/2:  -PI/2 means Vec(0,0,-1), PI/2
//		 means Vec(0,0,1)
//
//   colatitude: angle between the vector and the positive z-axis, with a
//		 range 0 - PI:  0 means Vec(0,0,1), PI means Vec(0,0,-1)
//
//   longitude:  angle between a projection of the vector in the x-y
//               plane and the x-axis, with a range -PI - PI:  0 means
//               Vec(1,0,0), -PI and PI both mean Vec(-1,0,0), PI/2
//               means Vec(0,-1,0), and -PI/2 means Vec(0,1,0).
//
//
// UV mappings:  Each of the above types of coordinates can also be
// represented in "UV" form, with two coordinates packed into a UV
// object, and scaled to the range 0-1.
//
//   UV-spherical: longitude and colatitude mapped to the range 0-1
//   		   via the following formulas:  u = longitude / (2*PI)
//   		   + 0.5, v = colatitude / PI
//
//   UV-latlong:   longitude and latitude mapped to the range 0-1 via
//   		   the following formulas:  u = longitude / (2*PI) +
//   		   0.5, v = (latitude / (PI/2)) + 0.5


#include "util/snogmath.h"
#include "vec.h"
#include "uv.h"


namespace snogray {


// z-axis-based vector to spherical-coordinate conversion functions

// Return the "latitude" of VEC, where the axis of the spherical
// coordinates is the z-axis.
//
// This is defined as the angle between the vector and the x-y plane,
// with a range of -PI/2 - PI/2; -PI/2 means Vec(0,0,-1), PI/2 means
// Vec(0,0,1).
//
template<typename T>
float
z_axis_latitude (const TVec<T> &vec)
{
  // This is equiv to: atan2 (vec.z, sqrt (vec.x * vec.x + vec.y * vec.y))
  // but asin is often faster than atan2 (and the division is negligible).
  return
    float (asin (clamp (vec.z
			/ sqrt (vec.x * vec.x
				+ vec.y * vec.y
				+ vec.z * vec.z),
			-1, 1)));
}

// Return the "colatitude" of VEC, where the axis of the spherical
// coordinates is the z-axis.
//
// This is defined as the angle between the vector and the positive
// z-axis, with a range of 0 - PI; 0 means Vec(0,0,1), PI means
// Vec(0,0,-1).
//
template<typename T>
float
z_axis_colatitude (const TVec<T> &vec)
{
  float lat = z_axis_latitude (vec);
  return (PIf / 2) - lat;
}

// Return the "longitude" of VEC, where the axis of the spherical
// coordinates is the z-axis.
//
// This is defined as the angle between a projection of the vector in
// the x-y plane and the x-axis, with a range -PI - PI:  0 means
// Vec(1,0,0), -PI and PI both mean Vec(-1,0,0), PI/2 means
// Vec(0,-1,0), and -PI/2 means Vec(0,1,0).
//
template<typename T>
float
z_axis_longitude (const TVec<T> &vec)
{
  return float (atan2 (-vec.y, vec.x));
}

// Return UV-encoded z-axis-based spherical coordinates for VEC.
//
// In the return value, u will be the longitude mapped to the
// range 0-1 using the formula u = longitude / (2*PI) + 0.5, and
// v will be the colatitude mapped to the range 0-1 using the
// formula v = colatitude / PI.
//
template<typename T>
UV
z_axis_spherical (const TVec<T> &vec)
{
  return UV (clamp01 (z_axis_longitude (vec) * INV_PIf * 0.5f + 0.5f),
	     clamp01 (z_axis_colatitude (vec) * INV_PIf));
}


// Return UV-encoded z-axis-based latitude-longitude coordinates
// for VEC.
//
// In the return value, u will be the longitude mapped to the
// range 0-1 using the formula u = longitude / (2*PI) + 0.5, and
// v will be the latitude mapped to the range 0-1 using the
// formula v = latitude / (PI/2) + 0.5.
//
template<typename T>
UV
z_axis_latlong (const TVec<T> &vec)
{
  return UV (clamp01 (z_axis_longitude (vec) * INV_PIf * 0.5f + 0.5f),
	     clamp01 (z_axis_latitude (vec) * INV_PIf + 0.5f));
}


// z-axis-based spherical/latlong vector construction functions.

// Return a vector corresponding to the z-axis-based spherical
// coordinates COS_COLAT and LNG.
//
// COS_COLAT is the _cosine_ of the colatitude (defined as the angle
// between the vector and the positive z-axis), with a range of 0 - 1.
// LNG is the longitude, defined as the angle between a projection of
// the vector in the x-y plane and the y-axis.
//
template<typename T>
Vec
z_axis_cos_spherical_to_vec (T cos_colat, T lng)
{
  T sin_lat = cos_colat;
  T cos_lat = sqrt (1 - sin_lat * sin_lat);
  T cos_lng = cos (lng);
  T sin_lng = sqrt (1 - cos_lng * cos_lng);
  if (lng < 0 || lng > T (PI))
    sin_lng = -sin_lng;		// sqrt formula returns abs value
  return Vec (dist_t (cos_lng * cos_lat),
	      dist_t (-sin_lng * cos_lat),
	      dist_t (sin_lat));
}

// Return a vector corresponding to the z-axis-based spherical
// coordinates COLAT and LNG.
//
// COLAT is the colatitude, defined as the angle between the vector
// and the positive z-axis, with a range of 0 - PI.  LNG is the
// longitude, defined as the angle between a projection of the vector
// in the x-y plane and the y-axis.
//
template<typename T>
inline Vec
z_axis_spherical_to_vec (T colat, T lng)
{
  return z_axis_cos_spherical_to_vec (cos (colat), lng);
}

// Return a vector corresponding to the z-axis-based spherical
// coordinates LAT and LNG.
//
// LAT is the latitude, defined as the angle between the vector and
// the x-y plane, with a range of -PI/2 - PI/2.  LNG is the longitude,
// defined as the angle between a projection of the vector in the x-y
// plane and the y-axis.
//
template<typename T>
inline Vec
z_axis_latlong_to_vec (T lat, T lng)
{
  T colat = T (PI / 2) - lat;
  return z_axis_spherical_to_vec (colat, lng);
}

// Return a vector corresponding to the UV-encoded z-axis-based
// spherical coordinates in COORDS.
//
// In COORDS, u is the longitude mapped to the range 0-1 using the
// formula u = longitude / (2*PI) + 0.5, and v is the colatitude
// mapped to the range 0-1 using the formula v = colatitude / PI.
//
inline Vec
z_axis_spherical_to_vec (const UV &coords)
{
  float lng = (coords.u - 0.5f) * PIf * 2;
  float colat = coords.v * PIf;
  return z_axis_spherical_to_vec (colat, lng);
}

// Return a vector corresponding to the UV-encoded z-axis-based
// latitude-longitude in COORDS.
//
// In COORDS, u is the longitude mapped to the range 0-1 using the
// formula u = longitude / (2*PI) + 0.5, and v is the latitude mapped
// to the range 0-1 using the formula v = latitude / (PI/2) + 0.5.
//
inline Vec
z_axis_latlong_to_vec (const UV &coords)
{
  float lng = (coords.u - 0.5f) * PIf * 2;
  float lat = (coords.v - 0.5f) * PIf;
  return z_axis_latlong_to_vec (lat, lng);
}


}

#endif // SNOGRAY_SPHERICAL_COORDS_H
