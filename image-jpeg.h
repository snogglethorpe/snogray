// image-jpeg.h -- JPEG format image handling
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_JPEG_H__
#define __IMAGE_JPEG_H__

#include "image-rgb-byte.h"

namespace Snogray {

struct JpegImageSinkParams : public RgbByteImageSinkParams
{
  JpegImageSinkParams (const ImageSinkParams &params)
    : RgbByteImageSinkParams (params)
  { }

  virtual ImageSink *make_sink () const;
};

}

#endif /* __IMAGE_JPEG_H__ */

// arch-tag: 354fa041-9c04-419b-a6e5-5c76fb3734cb
