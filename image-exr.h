// image-exr.h -- EXR format image handling
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_EXR_H__
#define __IMAGE_EXR_H__

#include "image.h"

namespace Snogray {

struct ExrImageSinkParams : public ImageFmtSinkParams
{
  ExrImageSinkParams (const ImageSinkParams &params)
    : ImageFmtSinkParams (params)
  {
    if (target_gamma != 0)
      params.error ("Target-gamma parameter not used in EXR format");
    if (params.quality)
      params.error ("Quality parameter not supported in EXR format");
  }
    
  virtual ImageSink *make_sink (unsigned width, unsigned height) const;
};

class ExrImageSourceParams : public ImageFmtSourceParams
{
public:
  ExrImageSourceParams (const ImageSourceParams &params)
    : ImageFmtSourceParams (params)
  { }
    
  virtual ImageSource *make_source () const;
};

}

#endif /* __IMAGE_EXR_H__ */

// arch-tag: aefc9982-81ff-4087-a25a-85c97c09475d
