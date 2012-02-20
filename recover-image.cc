// recover.cc -- Recovery of partial output images
//
//  Copyright (C) 2005-2007, 2010, 2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include "excepts.h"
#include "image-io.h"
#include "image-input.h"

#include "recover-image.h"


using namespace snogray;


// Initialize the output by opening the image file
// PARTIAL_IMAGE_FILENAME, using OUTPUT_FILENAME and OUTPUT_PARAMS to
// determine its format (because the PARTIAL_IMAGE_FILENAME may not be
// a normal image filename), and copying as many image rows as
// possible to DST; if a read-error is encountered, a small number of
// the last rows read are discarded, to guard against garbaged final
// lines.  The number of rows recovered is returned.
//
unsigned
snogray::recover_image (const std::string &partial_image_filename,
			const std::string &output_filename,
			const ValTable &output_params,
			ImageSampledOutput &dst)
{
  ValTable input_params = output_params;

  // Because PARTIAL_IMAGE_FILENAME may not be in a from which we can
  // deduce the image format, always use an explicit format parameter,
  // determined using OUTPUT_FILENAME and OUTPUT_PARAMS.
  //
  if (! input_params.contains ("format"))
    input_params.set ("format",
		      ImageIo::find_format (output_params, output_filename));

  int rows_recovered = 0;

  // We basically ignore any errors opening or closing
  // PARTIAL_IMAGE_FILENAME; they simply result in "no recovery".
  //
  try
    {
      // Open the input image.
      //
      ImageInput src (partial_image_filename, input_params);

      // Since we're copying previous final output values to DST, we need to
      // make sure that DST doesn't try to scale them.  To do this, just set
      // DST's intensity-scaling to identity values while doing recovery.
      //
      float old_intensity_scale = dst.intensity_scale;
      float old_intensity_power = dst.intensity_power;
      dst.intensity_scale = 1;
      dst.intensity_power = 1;

      ImageRow src_row (src.width);

      bool failed = false;
      while (!failed && rows_recovered < int (dst.height))
	{
	  try
	    {
	      src.read_row (src_row);
	    }
	  catch (...)
	    {
	      failed = true;
	    }

	  if (! failed)
	    {
	      ImageRow &dst_row = dst[rows_recovered].pixels;

	      for (unsigned x = 0; x < dst.width; x++)
		dst_row[x] = src_row[x];

	      dst[rows_recovered].weights.assign (dst.width, 1);
	    }

	  rows_recovered++;
	}

      // If we couldn't read the entire image, discard some of the final
      // rows we read (they should still be buffered in memory); this helps
      // with cases where the last few rows are garbage.
      //
      if (failed)
	for (unsigned i = 0;
	     i < RECOVER_DISCARD_ROWS && rows_recovered > 0;
	     i++)
	  dst[--rows_recovered].clear ();

      // Make sure the rows we recovered, which are still buffered in DST,
      // are flushed to the the output file.  It's important we do this,
      // because intensity-scaling in DST is applied when writing to the
      // output file, and we want the values we put into the buffer to get
      // our desired identity scaling.
      //
      dst.set_raw_min_y (rows_recovered);

      // Restore DST's intensity-scaling to what is desired during
      // rendering.
      //
      dst.intensity_scale = old_intensity_scale;
      dst.intensity_power = old_intensity_power;
    }
  catch (std::runtime_error &err) { /* nothing */ }

  return rows_recovered;
}


// arch-tag: b6d926f1-6b29-4f3e-a199-47a728600846
