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

#include "image-io.h"

namespace Snogray {

struct RgbeImageSinkParams : public ImageFmtSinkParams
{
  RgbeImageSinkParams (const ImageSinkParams &params)
    : ImageFmtSinkParams (params)
  {
    if (params.target_gamma && params.target_gamma != 1)
      params.error ("RGBE format does not use gamma correction");
  }
    
  virtual ImageSink *make_sink () const;
};

class RgbeImageSourceParams : public ImageFmtSourceParams
{
public:
  RgbeImageSourceParams (const ImageSourceParams &params)
    : ImageFmtSourceParams (params)
  { }
    
  virtual ImageSource *make_source () const;
};

}

#endif /* __IMAGE_RGBE_H__ */

// arch-tag: 40458d24-baf7-45ab-a68a-141a761eb39d
