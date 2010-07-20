// recover.cc -- Recovery of partial output images
//
//  Copyright (C) 2005, 2006, 2007, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "excepts.h"

#include "recover.h"


using namespace snogray;


// Initialize the output by reading as many image rows as possible from
// SRC and copying them to DST; if a read-error is encountered, a small
// number of the last rows read are discarded, to guard against garbaged
// final lines.  The number of rows recovered is returned.
//
// SRC is closed and deleted after recovering it.
//
unsigned
snogray::recover_image (ImageInput *src, ImageOutput &dst)
{
  ImageRow src_row (src->width);

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

	  dst[y].weights.assign (dst.width, 1);
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

  // Close the file-to-be-recovered, ignoring any errors in the process.
  //
  try { delete src; } catch (std::runtime_error &err) { }

  return y;
}


// arch-tag: b6d926f1-6b29-4f3e-a199-47a728600846
