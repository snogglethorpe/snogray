// image-scaled-output.h -- Output of scaled images
//
//  Copyright (C) 2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_IMAGE_SCALED_OUTPUT_H
#define SNOGRAY_IMAGE_SCALED_OUTPUT_H

#include "image-sampled-output.h"


namespace snogray {


// A subclass out ImageSampledOutput that makes writing scaled
// versions of images easy.
//
class ImageScaledOutput : ImageSampledOutput
{
public:

  // Create an ImageScaledOutput object for writing to FILENAME from a
  // source image size of SRC_WIDTH and SRC_HEIGHT.  PARAMS holds any
  // additional optional parameters.
  //
  ImageScaledOutput (const std::string &filename,
		     unsigned src_width, unsigned src_height,
		     const ValTable &params = ValTable::NONE);

  // Write the next row of the prescaled image.  It will be scaled
  // appropriately and written to the scaled output image.
  //
  void write_row (const ImageRow &row);

  // Inherit various stuff from our superclass.
  //
  using ImageSampledOutput::flush;
  using ImageSampledOutput::has_alpha_channel;
  using ImageSampledOutput::max_intens;
  using ImageSampledOutput::width;
  using ImageSampledOutput::height;
  using ImageSampledOutput::intensity_scale;
  using ImageSampledOutput::intensity_power;

private:

  // Calculate the width of the output image based on the size of the
  // source image and any other modifying image parameters.
  //
  static unsigned calc_dst_width (unsigned src_width, unsigned src_height,
				  const ValTable &params);

  // Calculate the height of the output image based on the size of the
  // source image and any other modifying image parameters.
  //
  static unsigned calc_dst_height (unsigned src_width, unsigned src_height,
				   const ValTable &params);

  // Calculate a modified set of image parameters for our
  // ImageSampledOutput superclass.
  //
  static ValTable calc_sampled_output_params (unsigned src_width,
					      unsigned src_height,
					      const ValTable &params);

  // Row number in the prescaled source-image space of the next row to
  // be written.
  //
  int cur_src_y;

  // Scale factors mapping from the source image size to the
  // destination image size.
  //
  float x_scale, y_scale;

  // If preclamping samples, the value to clamp to, otherwise 0.0f.
  //
  float preclamp;
};


}

#endif // SNOGRAY_IMAGE_SCALED_OUTPUT_H
