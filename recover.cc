// recover.cc -- Recovery of partial output images
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "excepts.h"

#include "recover.h"


using namespace Snogray;


// Initialize the output by reading as many image rows as possible from
// SRC and copying them to DST; if a read-error is encountered, a small
// number of the last rows read are discarded, to guard against garbaged
// final lines.  The number of rows recovered is returned.
//
// SRC is closed and deleted after recovering it.
//
unsigned
Snogray::recover_image (ImageInput *src, ImageOutput &dst)
{
  ImageRow src_row (src->width);

  // Make sure there's some number of rows buffered in memory, so we can
  // guard against errors.
  //
  dst.set_num_buffered_rows (16);

  int y = 0;
  bool failed = false;
  while (!failed && y < int (dst.height))
    {
      try
	{
	  src->read_row (src_row);
	}
      catch (...)
	{
	  failed = true;
	}

      if (! failed)
	{
	  ImageRow &dst_row = dst[y].pixels;

	  for (unsigned x = 0; x < dst.width; x++)
	    dst_row[x] = src_row[x];
	}

      y++;
    }

  // If we couldn't read the entire image, discard some of the final
  // rows we read (they should still be buffered in memory); this helps
  // with cases where the last few rows are garbage.
  //
  if (failed)
    for (unsigned i = 0; i < RECOVER_DISCARD_ROWS && y > 0; i++)
      dst[--y].clear ();

  // Set the lower output bound.  Rendering will ignore anything below
  // this bound, and treat it as the "image edge" (and so properly
  // handle rendering extra pixels to correctly deal with output filters
  // etc).
  //
  dst.set_min_y (y);

  // Close the file-to-be-recovered, ignoring any errors in the process.
  //
  try { delete src; } catch (std::runtime_error &err) { }

  return y;
}


// arch-tag: b6d926f1-6b29-4f3e-a199-47a728600846
