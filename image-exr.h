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

class ExrImageSinkParams : public ImageSinkParams
{
public:
  ExrImageSinkParams (const ImageGeneralSinkParams &params)
    : ImageSinkParams (params.width, params.height),
      file_name (params.file_name)
  {
    if (params.target_gamma != 0)
      params.error ("target-gamma not supported in EXR format");
  }
  ExrImageSinkParams (const char *_file_name,
		      unsigned _width, unsigned _height)
    : ImageSinkParams (_width, _height),
      file_name (_file_name)
  { }
    
  virtual ImageSink *make_sink () const;

  const char *file_name;
};

class ExrImageSourceParams : public ImageSourceParams
{
public:
  ExrImageSourceParams (const ImageGeneralSourceParams &params)
    : file_name (params.file_name)
  { }
  ExrImageSourceParams (const char *_file_name)
    : file_name (_file_name)
  { }
    
  virtual ImageSource *make_source () const;

  const char *file_name;
};

}

#endif /* __IMAGE_EXR_H__ */

// arch-tag: aefc9982-81ff-4087-a25a-85c97c09475d
