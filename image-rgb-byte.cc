// image-rgb-byte.cc -- Common code for image formats based on RGB byte arrays
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cmath>

#include "image-rgb-byte.h"

using namespace Snogray;

RgbByteImageSink::~RgbByteImageSink ()
{
  delete[] output_row;
}

void
RgbByteImageSink::write_row (const ImageRow &row)
{
  unsigned width = row.width;
  unsigned char *p = output_row;

  for (unsigned x = 0; x < width; x++)
    {
      const Color &col = row[x];
      *p++ = color_component_to_byte (col.red);
      *p++ = color_component_to_byte (col.green);
      *p++ = color_component_to_byte (col.blue);
    }

  write_row (output_row);
}

float
RgbByteImageSink::max_intens () const
{
  return 1;
}

// arch-tag: ee0370d1-7cdb-42d4-96e3-4cf7757cc2cf
