// image-rgbe.h -- Radiance RGBE / .hdr (aka .pic) format image handling
//
//  Copyright (C) 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_RGBE_H__
#define __IMAGE_RGBE_H__

#include "snogmath.h"
#include "image-io.h"


namespace Snogray {

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

    if (max_comp > 1e-32)
      {
	int iexp;
	float adj = frexp (max_comp, &iexp) * 255.9999 / max_comp;

	r = byte (adj * _r);
	g = byte (adj * _g);
	b = byte (adj * _b);
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
	return Color (scale * (r + 0.5), scale * (g + 0.5), scale * (b + 0.5));
      }
  }

  byte r, g, b, exp;
};


// Output

class RgbeImageSink : public ImageSink
{  
public:

  RgbeImageSink (const std::string &filename, unsigned width, unsigned height,
		 const Params &params = Params::NONE);

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
		   const Params &params = Params::NONE);

  virtual void read_row (ImageRow &row);

private:

  typedef unsigned char byte;

  void read_rle_component (byte RgbeColor::*component);

  std::ifstream inf;

  std::vector<RgbeColor> row_buf;
};

}

#endif /* __IMAGE_RGBE_H__ */


// arch-tag: 40458d24-baf7-45ab-a68a-141a761eb39d
