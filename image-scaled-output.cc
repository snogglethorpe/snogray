// image-scaled-output.cc -- Output of scaled images
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

#include "image-scaled-output.h"


using namespace snogray;


// Create an ImageScaledOutput object for writing to FILENAME from a
// source image size of SRC_WIDTH and SRC_HEIGHT.  PARAMS holds any
// additional optional parameters.
//
ImageScaledOutput::ImageScaledOutput (const std::string &filename,
				      unsigned src_width, unsigned src_height,
				      const ValTable &params)
  : ImageSampledOutput (filename,
			calc_dst_width (src_width, src_height, params),
			calc_dst_height (src_width, src_height, params),
			calc_sampled_output_params (src_width, src_height,
						    params)),
    cur_src_y (0),
    x_scale (float (width) / float (src_width)),
    y_scale (float (height) / float (src_height)),
    preclamp (
      params.get_bool ("preclamp", max_intens() != 0)
      ? max_intens ()
      : 0.0f)
{ }
			

// Write the next row of the prescaled image.  It will be scaled
// appropriately and written to the scaled output image.
//
void
ImageScaledOutput::write_row (const ImageRow &row)
{
  // Write to the output image, scaling as necessary.
  //
  for (unsigned x = 0; x < row.width; x++)
    {
      Tint sample = row[x];

      if (preclamp != 0.0f)
	sample  = sample.clamp (preclamp);

      add_sample ((x + 0.5f) * x_scale,
		  (cur_src_y + 0.5f) * y_scale,
		  sample);
    }

  cur_src_y++;
}


// Helper functions for the constructor

// Calculate the width of the output image based on the size of the
// source image and any other modifying image parameters.
//
unsigned
ImageScaledOutput::calc_dst_width (unsigned src_width, unsigned src_height,
				   const ValTable &params)
{
  unsigned width = params.get_uint ("width", 0);

  if (width == 0)
    {
      float aspect_ratio = float (src_width) / float (src_height);
      unsigned size = params.get_uint ("size", 0);

      if (size == 0)
	width = src_width;
      else
	width = (aspect_ratio >= 1) ? size : unsigned (size * aspect_ratio);
    }

  return width;
}

// Calculate the height of the output image based on the size of the
// source image and any other modifying image parameters.
//
unsigned
ImageScaledOutput::calc_dst_height (unsigned src_width, unsigned src_height,
				    const ValTable &params)
{
  unsigned height = params.get_uint ("height", 0);

  if (height == 0)
    {
      float aspect_ratio = float (src_width) / float (src_height);
      unsigned size = params.get_uint ("size", 0);

      if (size == 0)
	height = src_height;
      else
	height = (aspect_ratio <= 1) ? size : unsigned (size / aspect_ratio);
    }

  return height;
}

// Calculate a modified set of image parameters for our
// ImageSampledOutput superclass.
//
ValTable
ImageScaledOutput::calc_sampled_output_params (unsigned src_width,
					       unsigned src_height,
					       const ValTable &params)
{
  // Note that this method, which is intended to be used in the
  // constructor for initializing our superclass, duplicates
  // calculations later made for other arguments to the superclass
  // constructors; unfortunately, it's a bit annoying to create
  // temporary that gets calculated before initializing a superclass
  // (one would have to using a dummy superclass or something).
  // However, none of this is anything speed critical, so whatever.

  ValTable dst_params = params;

  unsigned dst_width = calc_dst_width (src_width, src_height, params);
  unsigned dst_height = calc_dst_height (src_width, src_height, params);

  if (dst_width == src_width && dst_height == src_height)
    {
      // Not doing any scaling, so turn off filtering unless some
      // filter has been explicitly specified.

      if (! dst_params.contains ("filter"))
	dst_params.set ("filter.type", "none");
    }
  else
    {
      // The output is being scaled, so set scaling-related parameters.

      // The scaling we apply during image conversion.
      //
      float x_scale = float (dst_width) / float (src_width);
      float y_scale = float (dst_height) / float (src_height);

      // If upscaling, make the filter width wide enough to cover the
      // output pixels.
      //
      if (x_scale > 1)
	dst_params.set ("filter.x_width_scale", x_scale);
      if (y_scale > 1)
	dst_params.set ("filter.y_width_scale", y_scale);
    }

  return dst_params;
}
