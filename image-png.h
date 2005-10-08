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

class PngImageSinkParams : public ImageSinkParams
{
public:
  static const float DEFAULT_TARG_GAMMA = 2.2;

  PngImageSinkParams (const ImageGeneralSinkParams &params)
    : ImageSinkParams (params.width, params.height),
      file_name (params.file_name), targ_gamma (params.target_gamma)
  { }
  PngImageSinkParams (const char *_file_name,
		      unsigned _width, unsigned _height,
		      float _targ_gamma = DEFAULT_TARG_GAMMA)
    : ImageSinkParams (_width, _height),
      file_name (_file_name), targ_gamma (_targ_gamma)
  { }
    
  virtual ImageSink *make_sink () const;

  const char *file_name;
  float targ_gamma;
};

}

#endif /* __IMAGE_PNG_H__ */

// arch-tag: fb6efedc-7ae1-433c-a703-59cafdfb9882
