// image.h -- Image datatype
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

#include <string>
#include <cmath>

#include "color.h"
#include "image-io.h"

namespace Snogray {

class Image
{
public:

  // Basic image constructor
  //
  Image (unsigned _width, unsigned _height)
    : width (_width), height (_height), pixels (new Color[_width * _height])
  { }
  ~Image ();

  // Constructors for an image loaded from a file
  //
  Image (const std::string &filename, const char *format = 0);
  Image (const std::string &filename, unsigned border);
  Image (const std::string &filename, const char *format, unsigned border);

  // Constructor for extracting a sub-image of BASE.  If W or H are 0, the
  // maximum available width or height is used.  Note that because of the
  // defaults, this is used as the copy-constructor.
  //
  Image (const Image &base, unsigned offs_x = 0, unsigned offs_y = 0,
	 unsigned w = 0, unsigned h = 0);

  const Color &operator() (unsigned x, unsigned y) const
  {
    return pixel (x, y);
  }
  Color &operator() (unsigned x, unsigned y)
  {
    return pixel (x, y);
  }

  const Color &pixel (unsigned x, unsigned y) const
  {
    return pixels[y * width + x];
  }
  Color &pixel (unsigned x, unsigned y)
  {
    return pixels[y * width + x];
  }

  void load (const std::string &filename, const char *format = 0, unsigned border = 0);

  void save (const ImageSinkParams &params);

  unsigned width, height;

private:

  Color *pixels;
};

}

#endif /* __IMAGE_H__ */

// arch-tag: 810abe16-816a-4fe0-a417-ab25c4f306ac
