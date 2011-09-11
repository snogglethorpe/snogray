// spherical-coords.h -- Mappings from spherical coordinates to/from vectors
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

#ifndef SNOGRAY_SPHERICAL_COORDS_H
#define SNOGRAY_SPHERICAL_COORDS_H

//
// This file contains functions for converting between spherical
// coordinates and vectors.  There are many such mappings, so the
// following definitions are somewhat arbitrarily chosen.
//
// y-axis coordinates:  defined on a sphere with its axis along the Y-axis
//
//   latitude:   angle between the vector and the x-z plane;
//		 range -PI - PI; -PI means Vec(0,-1,0), PI means Vec(0,1,0)
//
//   colatitude: angle between the vector and the positive y-axis;
//		 range 0 - PI; 0 means Vec(0,1,0), PI means Vec(0,-1,0)
//
//   longitude:  angle between projection of the vector in the x-z plane
//   		 and the z-axis; range -PI - PI; -PI and PI both mean
//   		 Vec(0,0,1), 0 means Vec(0,0,-1), and PI/2 means Vec(1,0,0)
//
//
// z-axis coordinates:  defined on a sphere with its axis along the z-axis
//
//   latitude:   angle between the vector and the x-y plane;
//		 range -PI - PI; -PI means Vec(0,0,-1), PI means Vec(0,0,1)
//
//   colatitude: angle between the vector and the positive z-axis;
//		 range 0 - PI; 0 means Vec(0,0,1), PI means Vec(0,0,-1)
//
//   longitude:  angle between projection of the vector in the x-y plane
//   		 and the y-axis; range -PI - PI; -PI and PI both mean
//   		 Vec(0,1,0), 0 means Vec(0,-1,0), and PI/2 means Vec(1,0,0)
//
//
// UV mappings:  Each of the above types of coordinates can also be
// represented in "UV" form, with two coordinates packed into a UV
// object, and scaled to the range 0-1.
//
//   UV-spherical:  longitude and colatitude mapped to the range 0-1 via
//   		 the following formulas:  u = longitude / (2*PI) + 0.5,
//   		 v = colatitude / PI
//
//   UV-latlong: longitude and colatitude mapped to the range 0-1 via
//   		 the following formulas:  u = longitude / (2*PI) + 0.5,
//   		 v = latitude / PI + 0.5


#include "snogmath.h"
#include "vec.h"
#include "uv.h"


namespace snogray {


// y-axis-based vector to spherical-coordinate conversion functions

// Return the latitude of VEC, where the axis of the spherical
// coordinates is the y-axis.  This is defined as the angle between
// the vector and the x-z plane, with a range of -PI - PI; -PI means
// Vec(0,-1,0), PI means Vec(0,1,0).
//
template<typename T>
static inline T
y_axis_latitude (const TVec<T> &vec)
{
  return atan2 (vec.y, sqrt (vec.x * vec.x + vec.z * vec.z));
}

// Return the colatitude of VEC, where the axis of the spherical
// coordinates is the y-axis.  This is defined as the angle between
// the vector and the positive y-axis, with a range of 0 - PI; 0
// means Vec(0,1,0), PI means Vec(0,-1,0).
//
template<typename T>
static inline T
y_axis_colatitude (const TVec<T> &vec)
{
  return atan2 (sqrt (vec.x * vec.x + vec.z * vec.z), vec.y);
}

// Return the longitude of VEC, where the axis of the spherical
// coordinates is the y-axis.  This is defined as the angle between
// a projection of the vector in the x-z plane and the z-axis, with
// a range -PI - PI; -PI and PI both mean Vec(0,0,1), 0 means
// Vec(0,0,-1), and PI/2 means Vec(1,0,0).
//
template<typename T>
static inline T
y_axis_longitude (const TVec<T> &vec)
{
  return atan2 (vec.x, vec.z);
}

// Return UV-encoded y-axis-based spherical coordinates for VEC.
// In the return value, u will be the longitude mapped to the
// range 0-1 using the formula u = longitude / (2*PI) + 0.5, and v
// will be the colatitude mapped to the range 0-1 using the
// formula v = colatitude / PI.
//
template<typename T>
static inline UV
y_axis_spherical (const TVec<T> &vec)
{
  return UV (clamp01 (y_axis_longitude (vec) * INV_PIf * 0.5f + 0.5f),
	     clamp01 (y_axis_colatitude (vec) * INV_PIf));
}

// Return UV-encoded y-axis-based latitude-longitude coordinates
// for VEC.  In the return value, u will be the longitude mapped to
// the range 0-1 using the formula u = longitude / (2*PI) + 0.5,
// and v will be the latitude mapped to the range 0-1 using the
// formula v = latitude / PI + 0.5.
//
template<typename T>
static inline UV
y_axis_latlong (const TVec<T> &vec)
{
  return UV (clamp01 (y_axis_longitude (vec) * INV_PIf * 0.5f + 0.5f),
	     clamp01 (y_axis_latitude (vec) * INV_PIf + 0.5f));
}


// z-axis-based vector to spherical-coordinate conversion functions

// Return the latitude of VEC, where the axis of the spherical
// coordinates is the z-axis.  This is defined as the angle between
// the vector and the x-y plane, with a range of -PI - PI; -PI means
// Vec(0,0,-1), PI means Vec(0,0,1).
//
template<typename T>
static inline T
z_axis_latitude (const TVec<T> &vec)
{
  return atan2 (vec.z, sqrt (vec.x * vec.x + vec.y * vec.y));
}

// Return the colatitude of VEC, where the axis of the spherical
// coordinates is the z-axis.  This is defined as the angle between
// the vector and the positive z-axis, with a range of 0 - PI; 0
// means Vec(0,0,1), PI means Vec(0,0,-1).
//
template<typename T>
static inline T
z_axis_colatitude (const TVec<T> &vec)
{
  return atan2 (sqrt (vec.x * vec.x + vec.y * vec.y), vec.z);
}

// Return the longitude of VEC, where the axis of the spherical
// coordinates is the z-axis.  This is defined as the angle between
// a projection of the vector in the x-y plane and the y-axis, with
// a range -PI - PI; -PI and PI both mean Vec(0,1,0), 0 means
// Vec(0,-1,0), and PI/2 means Vec(1,0,0).
//
template<typename T>
static inline T
z_axis_longitude (const TVec<T> &vec)
{
  return atan2 (vec.x, vec.y);
}

// Return UV-encoded z-axis-based spherical coordinates for VEC.
// In the return value, u will be the longitude mapped to the
// range 0-1 using the formula u = longitude / (2*PI) + 0.5, and v
// will be the colatitude mapped to the range 0-1 using the
// formula v = colatitude / PI.
//
template<typename T>
static inline UV
z_axis_spherical (const TVec<T> &vec)
{
  return UV (clamp01 (z_axis_longitude (vec) * INV_PIf * 0.5f + 0.5f),
	     clamp01 (z_axis_colatitude (vec) * INV_PIf));
}

// Return UV-encoded z-axis-based latitude-longitude coordinates
// for VEC.  In the return value, u will be the longitude mapped to
// the range 0-1 using the formula u = longitude / (2*PI) + 0.5,
// and v will be the latitude mapped to the range 0-1 using the
// formula v = latitude / PI + 0.5.
//
template<typename T>
static inline UV
z_axis_latlong (const TVec<T> &vec)
{
  return UV (clamp01 (z_axis_longitude (vec) * INV_PIf * 0.5f + 0.5f),
	     clamp01 (z_axis_latitude (vec) * INV_PIf + 0.5f));
}


// y-axis-based spherical-coordinate to vector conversion functions.

// Return a vector corresponding to the y-axis-based spherical
// coordinates COLAT and LNG.  COLAT is the colatitude, defined as
// the angle between the vector and the positive y-axis, with a
// range of 0 - PI.  LNG is the longitude, defined as the angle
// between a projection of the vector in the x-z plane and the
// z-axis.
//
template<typename T>
static inline TVec<T>
y_axis_spherical_to_vec (T colat, T lng)
{
  T sin_theta = sin (colat);
  return TVec<T> (sin (lng) * sin_theta, cos (colat), cos (lng) * sin_theta);
}

// Return a vector corresponding to the y-axis-based spherical
// coordinates COS_COLAT and LNG.  COS_COLAT is the _cosine_ of the
// colatitude (defined as the angle between the vector and the
// positive y-axis), with a range of 0 - 1.  LNG is the longitude,
// defined as the angle between a projection of the vector in the
// x-z plane and the z-axis.
//
template<typename T>
static inline TVec<T>
y_axis_cos_spherical_to_vec (T cos_colat, T lng)
{
  T sin_theta = sqrt (1 - cos_colat * cos_colat);
  return TVec<T> (sin (lng) * sin_theta, cos_colat, cos (lng) * sin_theta);
}

// Return a vector corresponding to the y-axis-based spherical
// coordinates LAT and LNG.  LAT is the latitude, defined as the
// angle between the vector and the x-z plane, with a range of -PI/2
// - PI/2.  LNG is the longitude, defined as the angle between a
// projection of the vector in the x-z plane and the z-axis.
//
template<typename T>
static inline TVec<T>
y_axis_latlong_to_vec (T lat, T lng)
{
  T cos_lat = cos (lat);
  return TVec<T> (sin (lng) * cos_lat, sin (lat), cos (lng) * cos_lat);
}

// Return a vector corresponding to the UV-encoded y-axis-based
// spherical coordinates in COORDS.
//
// In COORDS, u is the longitude mapped to the range 0-1 using the
// formula u = longitude / (2*PI) + 0.5, and v is the colatitude
// mapped to the range 0-1 using the formula v = colatitude / PI.
//
static inline Vec
y_axis_spherical_to_vec (const UV &coords)
{
  dist_t lng = (coords.u - 0.5f) * PIf * 2;
  dist_t colat = coords.v * PIf;
  return y_axis_spherical_to_vec (colat, lng);
}

// Return a vector corresponding to the UV-encoded y-axis-based
// latitude-longitude in COORDS.
//
// In COORDS, u is the longitude mapped to the range 0-1 using the
// formula u = longitude / (2*PI) + 0.5, and v is the latitude
// mapped to the range 0-1 using the formula v = latitude / PI + 0.5.
//
static inline Vec
y_axis_latlong_to_vec (const UV &coords)
{
  dist_t lng = (coords.u - 0.5f) * PIf * 2;
  dist_t lat = (coords.v - 0.5f) * PIf;
  return y_axis_latlong_to_vec (lat, lng);
}


// z-axis-based spherical/latlong vector construction functions.

// Return a vector corresponding to the z-axis-based spherical
// coordinates COLAT and LNG.  COLAT is the colatitude, defined as
// the angle between the vector and the positive z-axis, with a
// range of 0 - PI.  LNG is the longitude, defined as the angle
// between a projection of the vector in the x-y plane and the
// y-axis.
//
template<typename T>
static inline TVec<T>
z_axis_spherical_to_vec (T colat, T lng)
{
  T sin_theta = sin (colat);
  return TVec<T> (sin (lng) * sin_theta, cos (lng) * sin_theta, cos (colat));
}

// Return a vector corresponding to the z-axis-based spherical
// coordinates COS_COLAT and LNG.  COS_COLAT is the _cosine_ of the
// colatitude (defined as the angle between the vector and the
// positive z-axis), with a range of 0 - 1.  LNG is the longitude,
// defined as the angle between a projection of the vector in the
// x-y plane and the y-axis.
//
template<typename T>
static inline TVec<T>
z_axis_cos_spherical_to_vec (T cos_colat, T lng)
{
  T sin_theta = sqrt (1 - cos_colat * cos_colat);
  return TVec<T> (sin (lng) * sin_theta, cos (lng) * sin_theta, cos_colat);
}

// Return a vector corresponding to the z-axis-based spherical
// coordinates LAT and LNG.  LAT is the latitude, defined as the
// angle between the vector and the x-y plane, with a range of -PI/2
// - PI/2.  LNG is the longitude, defined as the angle between a
// projection of the vector in the x-y plane and the y-axis.
//
template<typename T>
static inline TVec<T>
z_axis_latlong_to_vec (T lat, T lng)
{
  T cos_lat = cos (lat);
  return TVec<T> (sin (lng) * cos_lat, cos (lng) * cos_lat, sin (lat));
}

// Return a vector corresponding to the UV-encoded z-axis-based
// spherical coordinates in COORDS.
//
// In COORDS, u is the longitude mapped to the range 0-1 using the
// formula u = longitude / (2*PI) + 0.5, and v is the colatitude
// mapped to the range 0-1 using the formula v = colatitude / PI.
//
static inline Vec
z_axis_spherical_to_vec (const UV &coords)
{
  dist_t lng = (coords.u - 0.5f) * PIf * 2;
  dist_t colat = coords.v * PIf;
  return z_axis_spherical_to_vec (colat, lng);
}

// Return a vector corresponding to the UV-encoded z-axis-based
// latitude-longitude in COORDS.
//
// In COORDS, u is the longitude mapped to the range 0-1 using the
// formula u = longitude / (2*PI) + 0.5, and v is the latitude
// mapped to the range 0-1 using the formula v = latitude / PI + 0.5.
//
static inline Vec
z_axis_latlong_to_vec (const UV &coords)
{
  dist_t lng = (coords.u - 0.5f) * PIf * 2;
  dist_t lat = (coords.v - 0.5f) * PIf;
  return z_axis_latlong_to_vec (lat, lng);
}


}

#endif // SNOGRAY_SPHERICAL_COORDS_H
