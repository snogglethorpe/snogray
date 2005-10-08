// image-png.h -- PNG format image handling
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_PNG_H__
#define __IMAGE_PNG_H__

#include "image.h"

namespace Snogray {

struct PngImageSinkParams : public ImageFmtSinkParams
{
  PngImageSinkParams (const ImageSinkParams &params)
    : ImageFmtSinkParams (params)
  { }

  virtual ImageSink *make_sink () const;
};

}

#endif /* __IMAGE_PNG_H__ */

// arch-tag: fb6efedc-7ae1-433c-a703-59cafdfb9882
