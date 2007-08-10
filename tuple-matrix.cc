// image.cc -- ImageContents datatype
//
//  Copyright (C) 2005, 2006, 2007  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>
#include <algorithm>

#include "globals.h"
#include "excepts.h"

#include "image-input.h"
#include "image-output.h"

#include "tuple-matrix.h"

using namespace snogray;


TupleMatrixData::TupleMatrixData (unsigned _tuple_len,
				  const std::string &filename, unsigned border)
  : tuple_len (_tuple_len)
{
  load (filename, ValTable::NONE, border);
}

TupleMatrixData::TupleMatrixData (unsigned _tuple_len,
				  const std::string &filename,
				  const ValTable &params, unsigned border)
  : tuple_len (_tuple_len)
{
  load (filename, params, border);
}

TupleMatrixData::TupleMatrixData (unsigned _tuple_len,
				  ImageInput &src, unsigned border)
  : tuple_len (_tuple_len)
{
  load (src, border);
}


// Color / tuple translation

// Return a color from the tuple at location X, Y; if the tuple length
// is not the same as a Color, only Color::TUPLE_LEN members are returned,
// and any missing components set to zero.
//
Color
TupleMatrixData::pixel (unsigned x, unsigned y) const
{
  Color col;

  const float *t = tuple (x, y);

  unsigned copy_limit = Color::NUM_COMPONENTS;
  if (tuple_len < Color::NUM_COMPONENTS)
    copy_limit = tuple_len;

  unsigned i;
  for (i = 0; i < copy_limit; i++)
    col[i] = t[i];
  for (; i < Color::TUPLE_LEN; i++)
    col[i] = 0;

  return col;
}

// Set the tuple at location X, Y from the color COL; if the tuple length
// is not the same as a Color, only the first TUPLE_LEN members are copied,
// and any missing components set to zero.
//
void
TupleMatrixData::set_pixel (unsigned x, unsigned y, const Color &col)
{
  float *t = tuple (x, y);

  unsigned copy_limit = Color::NUM_COMPONENTS;
  if (tuple_len < Color::NUM_COMPONENTS)
    copy_limit = tuple_len;

  unsigned i;
  for (i = 0; i < copy_limit; i++)
    t[i] = col[i];
  for (; i < tuple_len; i++)
    t[i] = 0;
}


// Input loading

void
TupleMatrixData::load (const std::string &filename, const ValTable &params,
		       unsigned border)
{
  ImageInput src (filename, params);

  // Loading a very large image can be slow (largely due to thrashing -- a
  // 6K x 3K image requires 216 MB of memory unpacked!), so tell the user
  // what we're doing.
  //
  bool emit_size_note = (!quiet && width * height > 1024 * 1024);
  if (emit_size_note)
    {
      std::string bn = filename;
      std::string::size_type last_slash = bn.find_last_of ("/");

      if (last_slash != std::string::npos)
	bn.erase (0, last_slash + 1);

      std::cout << "* loading large image: " << bn
		<< " (" << width << " x " << height << ", "
		<< (width * height * tuple_len * sizeof (float) / (1024 * 1024))
		<< " MB" << ")...";
      std::cout.flush ();
    }

  load (src, border);

  if (emit_size_note)
    {
      std::cout << "done" << std::endl;
      std::cout.flush ();
    }
}

void
TupleMatrixData::load (ImageInput &src, unsigned border)
{
  width = src.width + border * 2;
  height = src.height + border * 2;

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
TupleMatrixData::save (const std::string &filename, const ValTable &params)
  const
{
  ImageOutput out (filename, width, height, params);
  save (out);
}

void
TupleMatrixData::save (ImageOutput &out) const
{
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
