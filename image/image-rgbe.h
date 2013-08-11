// image-rgbe.h -- Radiance RGBE / .hdr (aka .pic) format image handling
//
//  Copyright (C) 2006-2007, 2010-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_IMAGE_RGBE_H
#define SNOGRAY_IMAGE_RGBE_H

#include <fstream>

#include "util/snogmath.h"
#include "image-io.h"


namespace snogray {

struct RgbeColor
{
  typedef unsigned char byte;

  static const int exp_offs = 128;

  RgbeColor () : r (0), g (0), b (0), exp (0) { }

  RgbeColor (const Color &col)
    : r (0), g (0), b (0), exp (0)
  {
    Color::component_t _r = col.r(), _g = col.g(), _b = col.b();
    Color::component_t max_comp = max (_r, max (_g, _b));

    if (max_comp > 1e-32f)
      {
	int iexp;
	float adj = frexp (max_comp, &iexp) * 255.9999f / max_comp;

	r = byte (adj * max (_r, 0.f));
	g = byte (adj * max (_g, 0.f));
	b = byte (adj * max (_b, 0.f));
	exp = iexp + exp_offs;
      }
  }

  operator Color () const
  {
    if (exp == 0)
      return 0;
    else
      {
	float scale = ldexp (1.0, int (exp) - (exp_offs + 8));
	return Color (scale * (r + 0.5f),
		      scale * (g + 0.5f),
		      scale * (b + 0.5f));
      }
  }

  byte r, g, b, exp;
};


// Output

class RgbeImageSink : public ImageSink
{  
public:

  RgbeImageSink (const std::string &filename, unsigned width, unsigned height,
		 const ValTable &params = ValTable::NONE);

  virtual void write_row (const ImageRow &row);

private:

  typedef unsigned char byte;

  static const unsigned MIN_RUN_LEN = 4;

  void write_rle_component (byte RgbeColor::*component);

  std::ofstream outf;

  std::vector<RgbeColor> row_buf;
};


// Input

class RgbeImageSource : public ImageSource
{  
public:

  RgbeImageSource (const std::string &filename,
		   const ValTable &params = ValTable::NONE);

  virtual void read_row (ImageRow &row);

private:

  typedef unsigned char byte;

  void read_rle_component (byte RgbeColor::*component);

  std::ifstream inf;

  std::vector<RgbeColor> row_buf;
};

}

#endif /* SNOGRAY_IMAGE_RGBE_H */


// arch-tag: 40458d24-baf7-45ab-a68a-141a761eb39d
