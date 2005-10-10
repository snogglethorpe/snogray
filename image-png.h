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

#include "image-byte-vec.h"

namespace Snogray {

struct PngImageSinkParams : public ByteVecImageSinkParams
{
  PngImageSinkParams (const ImageSinkParams &params)
    : ByteVecImageSinkParams (params)
  {
    if (params.quality)
      params.error ("Quality parameter not supported in PNG format");
  }

  virtual ImageSink *make_sink () const;
};

struct PngImageSourceParams : public ByteVecImageSourceParams
{
  PngImageSourceParams (const ImageSourceParams &params)
    : ByteVecImageSourceParams (params)
  { }

  virtual ImageSource *make_source () const;
};

}

#endif /* __IMAGE_PNG_H__ */

// arch-tag: fb6efedc-7ae1-433c-a703-59cafdfb9882
