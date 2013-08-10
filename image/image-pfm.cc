// image-pfm.cc -- PFM ("Portable Float Map") format image handling
//
//  Copyright (C) 2005-2008, 2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

//
// PFM is a floating-point image format used by "HDR Shop".
//
// It is basically an ASCII header followed by the raw raster, where
// each pixel is 1 or 3 IEEE floating point numbers in binary format.
//
// The first line of the header is a magic number, consisting of the
// characters "PF\n" for RGB, or "Pf\n" for grey-scale.  The next line
// is the ASCII decimal height and width separated by a space.  The 3rd
// and last line is a (ASCII, floating-point) "scale factor", with the
// added wrinkle that if the scale factor is negative, the raster uses
// little-endian IEEE floats, and if it's positive, the raster uses
// big-endian IEEE floats.
//
// A description can be found at:
//
//    http://netpbm.sourceforge.net/doc/pfm.html
//
// However it is not supported in official netpbm releases.
//


#include "excepts.h"

#include "image-pfm.h"

using namespace snogray;

static bool
little_endian ()
{
  union { unsigned long l; char c[4]; } u;
  u.l = 0x12345678;
  return u.c[0] == 0x78;
}


// Output

PfmImageSink::PfmImageSink (const std::string &filename,
			    unsigned width, unsigned height,
			    const ValTable &params)
  : ImageSink (filename, width, height, params),
    outf (filename.c_str(),
	  std::ios_base::out|std::ios_base::binary|std::ios_base::trunc),
    raster (width * height * 3), next_y (0)
{
  if (params.contains ("gamma"))
    open_err ("PFM format does not use gamma correction");

  outf << "PF\n" << width << " " << height << "\n";
  outf << (little_endian() ? "-1" : "1") << "\n";
}

PfmImageSink::~PfmImageSink ()
{
  outf.write (reinterpret_cast<char *>(&raster[0]), width * height * 12);
}

void
PfmImageSink::write_row (const ImageRow &row)
{
  float *p = &raster[width * (height - 1 - next_y) * 3];

  for (unsigned x = 0; x < row.width; x++)
    {
      const Color &col = row[x].alpha_scaled_color ();

      *p++ = col.r();
      *p++ = col.g();
      *p++ = col.b();
    }

  next_y++;
}


// Input

PfmImageSource::PfmImageSource (const std::string &filename,
				const ValTable &params)
  : ImageSource (filename, params),
    inf (filename.c_str(), std::ios_base::binary), next_y (0)
{
  char magic[10];

  inf.read (magic, 3);
  if (magic[0] != 'P' || magic[1] != 'F' || magic[2] != '\n')
    throw bad_format ("not a PMF file");

  float scale;

  inf >> width >> height;
  inf.get(); // skip one character (normally newline)

  inf >> scale;
  inf.get(); // skip one character (normally newline)

  bool file_little_endian = (scale < 0);

  byte_swap_floats = (file_little_endian != little_endian ());

  raster.resize (width * height * 3);

  inf.read (reinterpret_cast<char *>(&raster[0]), width * height * 12);
}

void
PfmImageSource::read_row (ImageRow &row)
{
  float *p = &raster[width * (height - 1 - next_y) * 3];

  for (unsigned x = 0; x < row.width; x++)
    {
      float r = maybe_byte_swap (*p++);
      float g = maybe_byte_swap (*p++);
      float b = maybe_byte_swap (*p++);
      row[x].set_rgb (r, g, b);
    }

  next_y++;
}


// arch-tag: 225e07ef-d906-4866-9120-b7d360506249
