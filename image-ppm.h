// image-ppm.h -- PPM format image handling
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_PPM_H__
#define __IMAGE_PPM_H__

#include "image.h"


// PPM has fixed gamma correction:  each sample is "...  proportional to
// the intensity of the CIE Rec. 709 red, green, and blue in the pixel,
// adjusted by the CIE Rec.  709 gamma transfer function.  (That
// transfer function specifies a gamma number of 2.2 and has a linear
// section for small intensities)."
//
#define IMAGE_PPM_GAMMA 2.2

// asdlkfjasf
#define _IMAGE_PPM_GAMMA_STRING "2.2"


namespace Snogray {

struct PpmImageSinkParams : public ImageFmtSinkParams
{
  PpmImageSinkParams (const ImageSinkParams &params)
    : ImageFmtSinkParams (params)
  {
    if (params.target_gamma && params.target_gamma != IMAGE_PPM_GAMMA)
      params.error ("PPM format uses a fixed gamma of "
		    _IMAGE_PPM_GAMMA_STRING);
  }
    
  virtual ImageSink *make_sink () const;
};

class PpmImageSourceParams : public ImageFmtSourceParams
{
public:
  PpmImageSourceParams (const ImageSourceParams &params)
    : ImageFmtSourceParams (params)
  { }
    
  virtual ImageSource *make_source () const;
};

}

#endif /* __IMAGE_PPM_H__ */

// arch-tag: 0b6f409a-0140-4dd4-933a-2e8c16541952
