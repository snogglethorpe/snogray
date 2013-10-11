// tangent-disk-sample.h -- Sample a disk tangent to a sphere
//
//  Copyright (C) 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_TANGENT_DISK_SAMPLE_H
#define SNOGRAY_TANGENT_DISK_SAMPLE_H

#include "uv.h"
#include "pos.h"
#include "vec.h"
#include "frame.h"
#include "disk-sample.h"


namespace snogray {


// Given a sphere defined by SPHERE_CENTER and SPHERE_RADIUS, sample
// over a disk with the same radius (SPHERE_RADIUS), whose center is
// tangent to the sphere in direction DIR from the sphere's center.
// PARAM is the sample parameter.
//
static inline Pos
tangent_disk_sample (const Pos &sphere_center, dist_t sphere_radius,
		     const Vec &dir, const UV &param)
{
  // FRAME is located at the center of the the sphere, and pointed in
  // the direction DIR (where the center of the disk should be tangent
  // to the sphere).
  //
  Frame frame (sphere_center, dir);

  // Choose a sample point on a disk with radius SPHERE_RADIUS.
  //
  coord_t px, py;
  disk_sample (sphere_radius, param, px, py);
  
  // Now move the point PX,PY on the disk, SCENE_RADIUS units away
  // along the z-axis in FRAME's coordinate system, and transform the
  // resulting position back to world coordinates.
  //
  return frame.from (Pos (px, py, sphere_radius));
}


}

#endif // SNOGRAY_TANGENT_DISK_SAMPLE_H
