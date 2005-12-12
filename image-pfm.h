// image-pfm.h -- PFM ("Portable Float Map") format image handling
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_PFM_H__
#define __IMAGE_PFM_H__

#include "image-io.h"

namespace Snogray {

struct PfmImageSinkParams : public ImageFmtSinkParams
{
  PfmImageSinkParams (const ImageSinkParams &params)
    : ImageFmtSinkParams (params)
  {
    if (params.target_gamma && params.target_gamma != 1)
      params.error ("PFM format does not use gamma correction");
  }
    
  virtual ImageSink *make_sink () const;
};

class PfmImageSourceParams : public ImageFmtSourceParams
{
public:
  PfmImageSourceParams (const ImageSourceParams &params)
    : ImageFmtSourceParams (params)
  { }
    
  virtual ImageSource *make_source () const;
};

}

#endif /* __IMAGE_PFM_H__ */

// arch-tag: d5e5373d-68c5-4086-bc9b-add7f13e3e26
