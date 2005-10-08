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

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "color.h"

namespace Snogray {

class Image
{
public:
  static const float DEFAULT_TARG_GAMMA = 2.2;

  Image (unsigned _width, unsigned _height,
	 float _targ_gamma = DEFAULT_TARG_GAMMA)
    : width (_width), height (_height),
      pixels (new Color[_width * _height]),
      targ_gamma (_targ_gamma), gamma_correction (1 / _targ_gamma)
  { }

  Image (const Image &src_image, unsigned aa_factor)
    : width (src_image.width / aa_factor),
      height (src_image.height / aa_factor),
      pixels (new Color[(src_image.width / aa_factor) * (src_image.height / aa_factor)]),
      targ_gamma (src_image.targ_gamma),
      gamma_correction (src_image.gamma_correction)
  {
    interpolate_pixels (src_image, aa_factor);
  }

  Color& operator() (unsigned x, unsigned y)
  {
    return pixels[x + y * width];
  }
  const Color& operator() (unsigned x, unsigned y) const
  {
    return pixels[x + y * width];
  }

  void write_png_file (const char *name);

  unsigned width, height;
  float targ_gamma, gamma_correction;

private:
  unsigned char color_component_to_byte (Color::component_t com);

  void interpolate_pixels (const Image &src_image, unsigned aa_factor);

  Color *pixels;
};

}

#endif /* __IMAGE_H__ */

// arch-tag: 43784b62-1eae-4938-a451-f4fdfb7db5bc
