// texture2.h -- 2d texture datatype
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __TEXTURE2_H__
#define __TEXTURE2_H__

#include <string>

#include "color.h"
#include "image.h"

namespace Snogray {

class Texture2
{
public:

  typedef float param_t;

  Texture2 (const std::string &filename)
    : image (filename),
      u_scale (image.width - 1), v_scale (image.height - 1)
  { }

  Color map (param_t u, param_t v);

  // A 2d texture is basically just an image with convenient mapping of
  // coordinates, and interpolation between pixels.
  //
  Image image;
  
  param_t u_scale, v_scale;
};

}

#endif /* __TEXTURE2_H__ */

// arch-tag: 0d92b346-d347-4025-aab0-cbaf594f6789
