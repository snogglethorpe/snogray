// snogdiff.cc -- Image-comparison utility
//
//  Copyright (C) 2005, 2006  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>

#include "snogmath.h"
#include "cmdlineparser.h"
#include "image-input.h"
#include "image-output.h"
#include "image-cmdline.h"

using namespace Snogray;
using namespace std;



static void
usage (CmdLineParser &clp, ostream &os)
{
  os << "Usage: " << clp.prog_name()
     << "[OPTION...] SRC_IMAGE_1 [SRC_IMAGE_2 [OUTPUT_IMAGE]]"
     << endl;
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
  "Output the difference of two images"
n
s IMAGE_INPUT_OPTIONS_HELP
n
s IMAGE_OUTPUT_OPTIONS_HELP
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
    IMAGE_OUTPUT_LONG_OPTIONS,
    CMDLINEPARSER_GENERAL_LONG_OPTIONS,
    { 0, 0, 0, 0 }
  };
  char short_options[] =
    IMAGE_OUTPUT_SHORT_OPTIONS
    IMAGE_INPUT_SHORT_OPTIONS
    CMDLINEPARSER_GENERAL_SHORT_OPTIONS;
  //
  CmdLineParser clp (argc, argv, short_options, long_options);

  // Parameters set from the command line
  //
  Params src_params, dst_params;

  // Parse command-line options
  //
  int opt;
  while ((opt = clp.get_opt ()) > 0)
    switch (opt)
      {
	IMAGE_OUTPUT_OPTION_CASES (clp, dst_params);
	IMAGE_INPUT_OPTION_CASES (clp, src_params);
	CMDLINEPARSER_GENERAL_OPTION_CASES (clp);
      }

  if (clp.num_remaining_args() < 1 || clp.num_remaining_args() > 3)
    {
      usage (clp, cerr);
      cerr << "Try `" << clp.prog_name() << " --help' for more information"
	   << endl;
      exit (10);
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

  // Open the output image using the resulting adjust size.
  //
  ImageOutput dst (clp.get_arg (), width, height, dst_params);

  // These are temp rows we use during reading
  //
  ImageRow row1 (width), row2 (width);

  // Copy input image to output image, doing any processing
  //
  for (unsigned y = 0; y < height; y++)
    {
      src1.read_row (row1);
      src2.read_row (row2);

      for (unsigned x = 0; x < width; x++)
	{
	  Color p = row1[x] - row2[x];

	  // We care about the absolute value of the difference
	  // (negative values aren't useful), so invert any negative
	  // components.
	  //
	  Color::component_t r = abs (p.r()), g = abs (p.g()), b = abs (p.b());
	  p.set_rgb (r, g, b);

	  dst.add_sample (x + 0.5, y + 0.5, p);
	}
    }
}

// arch-tag: 7e0ac89a-194f-4ebb-be2f-ca8714bca63c
