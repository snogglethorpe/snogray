// snoghilite.cc -- Add highlights to low-dynamic-range image
//
//  Copyright (C) 2012, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>
#include <cstring>

#include "util/string-funs.h"
#include "util/unique-ptr.h"
#include "cli/cmdlineparser.h"
#include "image/image-input.h"
#include "image/image-scaled-output.h"
#include "image/image-input-cmdline.h"
#include "image/image-scaled-output-cmdline.h"

using namespace snogray;



static void
usage (CmdLineParser &clp, std::ostream &os)
{
  os << "Usage: " << clp.prog_name()
     << " [OPTION...] INPUT_IMAGE_FILE OUTPUT_IMAGE_FILE" << std::endl;
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
  "Amplify image highlights"
n
s "  -t, --highlight-threshold=INTENS"
s "                             Use INTENS as the threshold for identifying"
s "                               image highlights (default 0.95)"
s "  -m, --highlight-scale=SCALE"
s "                             Scale image highlights by a factor of SCALE"
s "                               (default 5)"
n
s IMAGE_INPUT_OPTIONS_HELP
n
s IMAGE_SCALED_OUTPUT_OPTIONS_HELP
n
s CMDLINEPARSER_GENERAL_OPTIONS_HELP
n
s "The input image is copied to the output image, with all highlights"
s "scaled by a constant factor.  A \"highlight\" is any area of the image"
s "whose intensity exceeds a given threshold."
n
s "This is particularly useful for turning a low-dynamic-range"
s "environment-map into a HDR (high-dynamic-range) environment-map"
s "usable (although obviously not accurate) for scene lighting."
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
    { "highlight-threshold", required_argument, 0, 't' },
    { "highlight-scale", required_argument, 0, 'm' },
    IMAGE_INPUT_LONG_OPTIONS,
    IMAGE_SCALED_OUTPUT_LONG_OPTIONS,
    CMDLINEPARSER_GENERAL_LONG_OPTIONS,
    { 0, 0, 0, 0 }
  };
  char short_options[] =
    "t:m:"
    IMAGE_INPUT_SHORT_OPTIONS
    IMAGE_SCALED_OUTPUT_SHORT_OPTIONS
    CMDLINEPARSER_GENERAL_SHORT_OPTIONS;
  //
  CmdLineParser clp (argc, argv, short_options, long_options);

  // Parameters set from the command line
  //
  ValTable src_params, dst_params;
  float hl_thresh = 0.95, hl_scale = 5;

  // Parse command-line options
  //
  int opt;
  while ((opt = clp.get_opt ()) > 0)
    switch (opt)
      {
      case 't':
	hl_thresh = clp.float_opt_arg ();
	break;
      case 'm':
	hl_scale = clp.float_opt_arg ();
	break;

	IMAGE_INPUT_OPTION_CASES (clp, src_params);
	IMAGE_SCALED_OUTPUT_OPTION_CASES (clp, dst_params);
	CMDLINEPARSER_GENERAL_OPTION_CASES (clp);
      }

  if (clp.num_remaining_args() != 2)
    {
      usage (clp, std::cerr);
      clp.try_help_err ();
    }

  // Open the input image
  //
  ImageInput src (clp.get_arg(), src_params);

  // If the input has an alpha-channel, try to preserve it.
  //
  if (src.has_alpha_channel ())
    dst_params.set ("alpha_channel", true);

  // We catch any exceptions thrown while the output file is open (and
  // then just rethrow them), which ensures that all destructors are
  // called, and thus that the output file's buffers are flushed even
  // if an error occurs while processing.
  //
  // This is necessary because the C++ standard allows an unhandled
  // exception to call std::terminate immediately, without unwinding
  // the stack.
  try
    {
      // Open the output image.
      //
      std::string dst_name = clp.get_arg ();
      ImageScaledOutput dst (dst_name, src.width, src.height, dst_params);

      if (src.has_alpha_channel() && !dst.has_alpha_channel())
	std::cerr << clp.err_pfx()
		  << dst_name << ": warning: alpha-channel not preserved"
		  << std::endl;

      // Copy input image to output image, doing any processing
      //
      ImageRow row (src.width);
      for (unsigned y = 0; y < src.height; y++)
	{
	  src.read_row (row);
	  
	  for (unsigned x = 0; x < src.width; x++)
	    for (unsigned cc = 0; cc < Color::NUM_COMPONENTS; cc++)
	      if (row[x].color[cc] > hl_thresh)
		row[x].color[cc] *= hl_scale;

	  dst.write_row (row);
	}
    }
  catch (...) { throw; }
}
