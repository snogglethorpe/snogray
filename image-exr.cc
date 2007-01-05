// image-exr.cc -- EXR format image handling
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "image-exr.h"

using namespace snogray;

void
ExrImageSink::write_row (const ImageRow &row)
{
  for (unsigned x = 0; x < row.width; x++)
    {
      const Color &col = row[x];
      Imf::Rgba rgba (col.r(), col.g(), col.b(), 1);
      row_buf[x] = rgba;
    }

  outf.setFrameBuffer (&row_buf[0] - cur_y * row.width, 1, row.width);
  outf.writePixels ();

  cur_y++;
}

void
ExrImageSource::read_row (ImageRow &row)
{
  outf.setFrameBuffer (&row_buf[0] - cur_y * row.width, 1, row.width);
  outf.readPixels (cur_y);

  for (unsigned x = 0; x < row.width; x++)
    {
      const Imf::Rgba &rgba = row_buf[x];
      row[x].set_rgb (rgba.r, rgba.g, rgba.b);
    }

  cur_y++;
}


// arch-tag: a6c557de-fa39-4773-8357-dee599502a47
