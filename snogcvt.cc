// snogcvt.cc -- Image-type conversion utility
//
//  Copyright (C) 2005-2012  Miles Bader <miles@gnu.org>
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

#include "cmdlineparser.h"
#include "image-input.h"
#include "image-scaled-output.h"
#include "image-input-cmdline.h"
#include "image-scaled-output-cmdline.h"
#include "string-funs.h"
#include "unique-ptr.h"

using namespace snogray;
using namespace std;



static void
usage (CmdLineParser &clp, ostream &os)
{
  os << "Usage: " << clp.prog_name()
     << " [OPTION...] [SOURCE_IMAGE [OUTPUT_IMAGE]]" << endl;
}

static void
help (CmdLineParser &clp, ostream &os)
{
  usage (clp, os);

  // These macros just makes the source code for help output easier to line up
  //
#define s  << endl <<
#define n  << endl

  os <<
  "Change the format of or transform an image file"
n
s IMAGE_INPUT_OPTIONS_HELP
n
s IMAGE_SCALED_OUTPUT_OPTIONS_HELP
n
s CMDLINEPARSER_GENERAL_OPTIONS_HELP
n
s "If no filenames are given, standard input or output is used.  Input/output"
s "image formats are guessed using the corresponding filenames when possible"
s "(using the file's extension)."
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
    IMAGE_INPUT_LONG_OPTIONS,
    IMAGE_SCALED_OUTPUT_LONG_OPTIONS,
    CMDLINEPARSER_GENERAL_LONG_OPTIONS,
    { 0, 0, 0, 0 }
  };
  char short_options[] =
    IMAGE_INPUT_SHORT_OPTIONS
    IMAGE_SCALED_OUTPUT_SHORT_OPTIONS
    CMDLINEPARSER_GENERAL_SHORT_OPTIONS;
  //
  CmdLineParser clp (argc, argv, short_options, long_options);

  // Parameters set from the command line
  //
  ValTable src_params, dst_params;

  // Parse command-line options
  //
  int opt;
  while ((opt = clp.get_opt ()) > 0)
    switch (opt)
      {
	IMAGE_INPUT_OPTION_CASES (clp, src_params);
	IMAGE_SCALED_OUTPUT_OPTION_CASES (clp, dst_params);
	CMDLINEPARSER_GENERAL_OPTION_CASES (clp);
      }

  if (clp.num_remaining_args() > 2)
    {
      usage (clp, cerr);
      cerr << "Try `" << clp.prog_name() << " --help' for more information"
	   << endl;
      exit (10);
    }

  // Open the input image
  //
  ImageInput src (clp.get_arg(), src_params);

  // If the input has an alpha-channel, try to preserve it.
  //
  if (src.has_alpha_channel ())
    dst_params.set ("alpha-channel", true);

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
      ImageRow src_row (src.width);
      for (unsigned y = 0; y < src.height; y++)
	{
	  src.read_row (src_row);
	  dst.write_row (src_row);
	}
    }
  catch (...) { throw; }
}
