// disk-sample.h -- Disk sample distribution
//
//  Copyright (C) 2006-2007, 2010-2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_DISK_SAMPLE_H
#define SNOGRAY_DISK_SAMPLE_H

#include "snogmath.h"
#include "coords.h"
#include "uv.h"


namespace snogray {


// Sample a disk centered at the origin with radius RADIUS using the
// parameter PARAM, returning the sample coordinates in DX and DY.
//
static inline void
disk_sample (dist_t radius, const UV &param, dist_t &dx, dist_t &dy)
{
  float u = 2 * param.u - 1;
  float v = 2 * param.v - 1;

  if (u == 0 && v == 0)
    dx = dy = 0.0;
  else
    {
      float r, theta;

      if (u >= -v)
	{
	  if (u > v)
	    {
	      r = u;
	      theta = (v > 0) ? v / r : 8 + v / r;
	    }
	  else
	    {
	      r = v;
	      theta = 2 - u / r;
	    }
	}
      else
	{
	  if (u <= v)
	    {
	      r = -u;
	      theta = 4 - v / r;
	    }
	  else
	    {
	      r = -v;
	      theta = 6 + u / r;
	    }
	}

      theta *= PIf / 2;

      dx = radius * dist_t (r * cos (theta));
      dy = radius * dist_t (r * sin (theta));
    }
}

// Sample a disk centered at the origin with radius 1 using the
// parameter PARAM, returning the sample coordinates in DX and DY.
//
static inline void
disk_sample (const UV &param, dist_t &dx, dist_t &dy)
{
  return disk_sample (1, param, dx, dy);
}


}

#endif // SNOGRAY_DISK_SAMPLE_H


// arch-tag: be2d668d-431a-4354-8283-823cdd0fe6bc
