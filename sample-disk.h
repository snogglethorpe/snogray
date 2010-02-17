// sample-disk.h -- Disk sample distribution
//
//  Copyright (C) 2006, 2007, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SAMPLE_DISK_H__
#define __SAMPLE_DISK_H__

#include "snogmath.h"
#include "uv.h"


namespace snogray {


// Convert uniformly distributed (U, V) parameters with a range of 0-1
// to samples uniformly distributed on a disk, where DX and DY are x/y
// offsets on the disk with range -1 - 1.
//
inline void
sample_disk (const UV &param, float &dx, float &dy)
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
	      theta = (v > 0.0) ? v / r : 8 + v / r;
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

      theta *= PIf / 2.f;

      dx = r * cos (theta);
      dy = r * sin (theta);
    }
}


}

#endif // __SAMPLE_DISK_H__


// arch-tag: be2d668d-431a-4354-8283-823cdd0fe6bc
