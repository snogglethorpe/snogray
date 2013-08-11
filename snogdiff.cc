// snogdiff.cc -- Image-comparison utility
//
//  Copyright (C) 2005-2008, 2012, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>
#include <iomanip>

#include "snogmath.h"
#include "cmdlineparser.h"
#include "unique-ptr.h"
#include "image/image-input.h"
#include "image/image-scaled-output.h"
#include "imagecli/image-input-cmdline.h"
#include "imagecli/image-scaled-output-cmdline.h"

using namespace snogray;



static void
usage (CmdLineParser &clp, std::ostream &os)
{
  os << "Usage: " << clp.prog_name()
     << " [OPTION...] SRC_IMAGE_1 SRC_IMAGE_2 [OUTPUT_IMAGE]"
     << std::endl;
}

static void
help (CmdLineParser &clp, std::ostream &os)
{
  usage (clp, os);

  // These macros just makes the source code for help output easier to line up
  //
#define s  << std::endl <<
#define n  << std::endl

  os <<
  "Output the difference of two images"
n
s "  -d, --delta=THRESH         Set delta threshold for \"identical\" images"
s "  -m, --mse=THRESH           Set MSE threshold for \"identical\" images"
s "  -q, --quiet                Don't print image statistics"
n
s IMAGE_INPUT_OPTIONS_HELP
n
s IMAGE_SCALED_OUTPUT_OPTIONS_HELP
n
s CMDLINEPARSER_GENERAL_OPTIONS_HELP
n
s "In addition to producing a difference image (when an output filename"
s "is specified), if the images were different some image-comparison"
s "statistics are printed on stdout."
n
s "The exit status is zero (\"success\") if the images were identical,"
s "and non-zero otherwise."
n
s "Input/output image formats are guessed using the corresponding filenames"
s "(using the files' extensions)."
n
    ;

#undef s
#undef n
}


int main (int argc, char *const *argv)
{
  // Command-line option specs
  //
  static struct option long_options[] = {
    { "delta",	required_argument, 0, 'd' },
    { "mse",	required_argument, 0, 'm' },
    { "quiet",	no_argument, 0, 'q' },
    IMAGE_INPUT_LONG_OPTIONS,
    IMAGE_SCALED_OUTPUT_LONG_OPTIONS,
    CMDLINEPARSER_GENERAL_LONG_OPTIONS,
    { 0, 0, 0, 0 }
  };
  char short_options[] =
    "d:m:q"
    IMAGE_INPUT_SHORT_OPTIONS
    IMAGE_SCALED_OUTPUT_SHORT_OPTIONS
    CMDLINEPARSER_GENERAL_SHORT_OPTIONS;
  //
  CmdLineParser clp (argc, argv, short_options, long_options);

  // Parameters set from the command line
  //
  ValTable src_params, dst_params;

  // Image comparison parameters.
  //
  double delta_thresh = 0, mse_thresh = 0;
  bool quiet = false;

  // Parse command-line options
  //
  int opt;
  while ((opt = clp.get_opt ()) > 0)
    switch (opt)
      {
      case 'd': delta_thresh = clp.float_opt_arg (); break;
      case 'm': mse_thresh = clp.float_opt_arg (); break;
      case 'q': quiet = true; break;

	IMAGE_INPUT_OPTION_CASES (clp, src_params);
	IMAGE_SCALED_OUTPUT_OPTION_CASES (clp, dst_params);
	CMDLINEPARSER_GENERAL_OPTION_CASES (clp);
      }

  if (clp.num_remaining_args() < 2 || clp.num_remaining_args() > 3)
    {
      usage (clp, std::cerr);
      clp.try_help_err ();
    }

  // Open the input images
  //
  ImageInput src1 (clp.get_arg(), src_params);
  ImageInput src2 (clp.get_arg(), src_params);

  // We get the output image's size from the input image
  //
  unsigned width = src1.width;
  unsigned height = src1.height;

  if (src2.width != width || src2.height != height)
    clp.err ("Input images must be the same size");

  // The output image.
  //
  UniquePtr<ImageScaledOutput> dst;

  // The output image is optional, so only create if a name was given.
  //
  if (clp.num_remaining_args () == 1)
    dst.reset (new ImageScaledOutput (clp.get_arg (),
				      width, height, dst_params));;

  // These are temporary image rows used during processing.
  //
  ImageRow row1 (width), row2 (width), dst_row (width);

  // Statistics on input images: sum of all color-component values in
  // both images, and the sum of their differences squared.
  //
  double sum1 = 0, sum2 = 0;
  double sum_diff_sq = 0;

  // Copy input image to output image, doing any processing
  //
  for (unsigned y = 0; y < height; y++)
    {
      src1.read_row (row1);
      src2.read_row (row2);

      for (unsigned x = 0; x < width; x++)
	{
	  Color col1 = row1[x].alpha_scaled_color();
	  Color col2 = row2[x].alpha_scaled_color();

	  for (unsigned c = 0; c < Color::NUM_COMPONENTS; c++)
	    {
	      double val1 = col1[c];
	      double val2 = col2[c];

	      sum1 += val1;
	      sum2 += val2;
	      sum_diff_sq += (val1 - val2) * (val1 - val2);
	    }

	  dst_row[x] = abs (col1 - col2);
	}

      if (dst)
	dst->write_row (dst_row);
    }

  // Calculate image statistics.
  //
  double num_values = width * height * Color::NUM_COMPONENTS;
  double avg1 = sum1 / num_values;
  double avg2 = sum2 / num_values;
  double avg_delta = (std::abs (avg1 - avg2) / std::min (avg1, avg2));
  double mse = sum_diff_sq / num_values;

  // True if the images are considered "different."
  //
  bool different
    = (((delta_thresh > 0 || mse_thresh == 0) && avg_delta > delta_thresh)
       || ((mse_thresh > 0 || delta_thresh == 0) && mse > mse_thresh));

  // Print image statistics, but only if images differed.
  //
  if (different && !quiet)
    std::cout << std::fixed
	      << std::setprecision (6)
	      << "* avg1 = " << avg1
	      << ", avg2 = " << avg2
	      << std::setprecision (8)
	      << ", avg_delta = " << avg_delta
	      << ", mse = " << mse
	      << std::endl;

  exit (different ? 10 : 0);
}

// arch-tag: 7e0ac89a-194f-4ebb-be2f-ca8714bca63c
