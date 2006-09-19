// sample-disk.h -- Disk sample distribution
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __SAMPLE_DISK_H__
#define __SAMPLE_DISK_H__

#include "snogmath.h"


namespace Snogray {


// Convert uniformly distributed (U, V) parameters with a range of 0-1
// to samples uniformly distributed on a disk, where DX and DY are x/y
// offsets on the disk with range -1 - 1.
//
inline void
sample_disk (float u, float v, float &dx, float &dy)
{
  u = 2 * u - 1;
  v = 2 * v - 1;

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

      theta *= M_PIf / 4.f;

      dx = r * cos (theta);
      dy = r * sin (theta);
    }
}


}

#endif // __SAMPLE_DISK_H__


// arch-tag: be2d668d-431a-4354-8283-823cdd0fe6bc
