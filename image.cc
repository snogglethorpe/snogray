// image.cc -- Image datatype
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "image-io.h"
#include "excepts.h"

#include "image.h"

using namespace Snogray;

Image::Image (const std::string &filename, const char *format)
  : pixels (0)
{
  load (filename, format);
}

Image::Image (const std::string &filename, unsigned border)
  : pixels (0)
{
  load (filename, 0, border);
}

Image::Image (const std::string &filename, const char *format, unsigned border)
  : pixels (0)
{
  load (filename, format, border);
}

Image::~Image ()
{
  delete[] pixels;
}


// Input loading

void
Image::load (const std::string &filename, const char *format, unsigned border)
{
  ImageInput src (filename, format);

  width = src.width + border * 2;
  height = src.height + border * 2;

  if (pixels)
    delete[] pixels;

  pixels = new Color[width * height];

  ImageRow row (src.width);

  for (unsigned y = 0; y < src.height; y++)
    {
      src.read_row (row);

      for (unsigned x = 0; x < src.width; x++)
	pixel (x + border, y + border) = row[x];

      for (unsigned b = 0; b < border; b++)
	{
	  pixel (b, y + border) = 0;
	  pixel (width - b, y + border) = 0;
	}
    }
}

void
Image::save (const ImageSinkParams &params) const
{
  ImageOutput dst (params);

  for (unsigned y = 0; y < params.height; y++)
    {
      ImageRow &row = dst.next_row ();

      for (unsigned x = 0; x < params.width; x++)
	row[x] = pixel (x, y);
    }
}



// Constructor for extracting a sub-image of BASE
//
Image::Image (const Image &base, unsigned offs_x, unsigned offs_y,
	      unsigned w, unsigned h)
{
  if (offs_x + w > base.width || offs_y + h > base.height)
    throw std::runtime_error ("sub-image out of bounds");

  if (w == 0)
    w = base.width - offs_x;
  if (h == 0)
    h = base.height - offs_y;

  width = w;
  height = h;

  pixels = new Color[w * h];

  for (unsigned y = 0; y < h; y++)
    for (unsigned x = 0; x < w; x++)
      pixel (x, y) = base.pixel (x + offs_x, y + offs_y);
}


// arch-tag: da22c1bc-101a-4b6e-a7e6-1db2676ea923
