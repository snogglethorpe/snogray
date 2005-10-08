// image-exr.cc -- EXR format image handling
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <ImfRgbaFile.h>

#include "image-exr.h"

using namespace Snogray;

class ExrImageSink : public ImageSink
{  
public:
  ExrImageSink (const ExrImageSinkParams &params)
    : outf (params.file_name, params.width, params.height),
      row_buf (new Imf::Rgba[params.width]), cur_y (0)
  {
  }
  ~ExrImageSink ();

  virtual void write_row (const ImageRow &row);

private:
  Imf::RgbaOutputFile outf;
  Imf::Rgba *row_buf;
  unsigned cur_y;
};

ExrImageSink::~ExrImageSink ()
{
  delete[] row_buf;
}

void
ExrImageSink::write_row (const ImageRow &row)
{
  for (unsigned x = 0; x < row.width; x++)
    {
      const Color &col = row[x];
      Imf::Rgba rgba (col.red, col.green, col.blue, 1);
      row_buf[x] = rgba;
    }

  outf.setFrameBuffer (row_buf - cur_y * row.width, 1, row.width);
  outf.writePixels ();

  cur_y++;
}

ImageSink *
ExrImageSinkParams::make_sink () const
{
  return new ExrImageSink (*this);
}

// arch-tag: a6c557de-fa39-4773-8357-dee599502a47
