// image.cc -- ImageContents datatype
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>

#include "excepts.h"
#include "image-input.h"
#include "image-output.h"

#include "tuple-matrix.h"

using namespace Snogray;


TupleMatrixData::TupleMatrixData (unsigned _tuple_len,
				  const std::string &filename, unsigned border)
  : tuple_len (_tuple_len)
{
  load (filename, Params::NONE, border);
}

TupleMatrixData::TupleMatrixData (unsigned _tuple_len,
				  const std::string &filename,
				  const Params &params, unsigned border)
  : tuple_len (_tuple_len)
{
  load (filename, params, border);
}


// Color / tuple translation

// Return a color from the tuple at location X, Y (if the tuple length
// is not three, an appropriate translation is done).
//
Color
TupleMatrixData::pixel (unsigned x, unsigned y) const
{
  const float *t = tuple (x, y);

  if (tuple_len >= 3)
    return Color (t[0], t[1], t[2]);
  else if (tuple_len == 1)
    return Color (t[0], t[0], t[0]);
  else // tuple_len == 2
    return Color (t[0], t[1], 0);
}

// Set the tuple at location X, Y from the color COL (if the tuple
// length is not three, an appropriate translation is done).
//
void
TupleMatrixData::set_pixel (unsigned x, unsigned y, const Color &col)
{
  float *t = tuple (x, y);

  t[0] = col.r ();

  if (tuple_len > 1)
    {
      t[1] = col.g ();

      if (tuple_len > 2)
	t[2] = col.b ();
    }
}


// Input loading

void
TupleMatrixData::load (const std::string &filename, const Params &params,
		       unsigned border)
{
  ImageInput src (filename, params);

  width = src.width + border * 2;
  height = src.height + border * 2;

  // Loading a very large image can be slow (largely due to thrashing -- a
  // 6K x 3K image requires 216 MB of memory unpacked!), so tell the user
  // what we're doing.
  //
  if (width * height > 1024 * 1024)
    {
      std::string bn = filename;
      unsigned last_slash = bn.find_last_of ("/");

      if (last_slash != std::string::npos)
	bn.erase (0, last_slash + 1);

      std::cout << "* loading large image: " << bn
		<< " (" << width << " x " << height << ", "
		<< (width * height * tuple_len * sizeof (float) / (1024 * 1024))
		<< " MB" << ")" << std::endl;
    }

  data.resize (tuple_len * width * height);

  ImageRow row (src.width);

  for (unsigned y = 0; y < src.height; y++)
    {
      src.read_row (row);

      for (unsigned x = 0; x < src.width; x++)
	set_pixel (x + border, y + border, row[x]);

      for (unsigned b = 0; b < border; b++)
	{
	  set_pixel (b, y + border, 0);
	  set_pixel (width - b, y + border, 0);
	}
    }
}

void
TupleMatrixData::save (const std::string &filename, const Params &params) const
{
  ImageOutput out (filename, width, height, params);

  for (unsigned y = 0; y < height; y++)
    {
      ImageRow &row = out[y].pixels;

      for (unsigned x = 0; x < width; x++)
	row[x] = pixel (x, y);
    }
}



// Constructor for extracting a sub-image of BASE
//
TupleMatrixData::TupleMatrixData (unsigned _tuple_len,
				  const TupleMatrixData &base,
				  unsigned offs_x, unsigned offs_y,
				  unsigned w, unsigned h)
  : tuple_len (_tuple_len)
{
  if (offs_x + w > base.width || offs_y + h > base.height)
    throw std::runtime_error ("sub-image out of bounds");

  if (w == 0)
    w = base.width - offs_x;
  if (h == 0)
    h = base.height - offs_y;

  width = w;
  height = h;

  data.resize (tuple_len * w * h);

  for (unsigned y = 0; y < h; y++)
    for (unsigned x = 0; x < w; x++)
      set_pixel (x, y, base.pixel (x + offs_x, y + offs_y));
}


// arch-tag: da22c1bc-101a-4b6e-a7e6-1db2676ea923
