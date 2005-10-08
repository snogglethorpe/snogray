// image-rgb-byte.h -- Common code for image formats based on RGB byte arrays
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_RGB_BYTE_H__
#define __IMAGE_RGB_BYTE_H__

#include "image.h"

namespace Snogray {

struct RgbByteImageSinkParams : public ImageFmtSinkParams
{
  RgbByteImageSinkParams (const ImageSinkParams &params)
    : ImageFmtSinkParams (params)
  {
    if (! target_gamma)
      target_gamma = ImageSinkParams::DEFAULT_TARGET_GAMMA;
  }
};

class RgbByteImageSink : public ImageSink
{
public:

  RgbByteImageSink (const RgbByteImageSinkParams &params)
    : ImageSink (params),
      gamma_correction (params.target_gamma == 0 ? 0 : 1 / params.target_gamma),
      output_row (new unsigned char[params.width * 3])
  { }
  ~RgbByteImageSink ();

  // We define these
  virtual void write_row (const ImageRow &row);
  virtual float max_intens () const;

  // Subclasses can define this instead of the generic write_row
  virtual void write_row (const unsigned char *rgb_bytes) = 0;

  // Floating-point to byte conversion
  unsigned char color_component_to_byte (Color::component_t com)
  {
    if (com < 0)
      return 0;

    if (gamma_correction != 0)
      com = powf (com, gamma_correction);

    if (com >= 0.9999)
      return 255;
    else
      return (unsigned char)(256.0 * com);
  }

  // Explicit gamma-correction factor
  float gamma_correction;

private:

  // A single row of bytes we use as temporary storage during output
  unsigned char *output_row;
};

}

#endif /* __IMAGE_RGB_BYTE_H__ */

// arch-tag: e442d880-ba85-423e-8b1c-e3c4d9500528
