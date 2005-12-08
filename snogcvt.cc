// snogcvt.cc -- Image-type conversion utility
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>

#include "cmdlineparser.h"
#include "image-io.h"
#include "image-cmdline.h"

using namespace Snogray;
using namespace std;



static void
usage (CmdLineParser &clp, ostream &os)
{
  os << "Usage: " << clp.prog_name()
     << "[OPTION...] [SOURCE_IMAGE [OUTPUT_IMAGE]]" << endl;
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
  ImageCmdlineSourceParams src_image_params (clp);
  ImageCmdlineSinkParams dst_image_params (clp);

  // Parse command-line options
  //
  int opt;
  while ((opt = clp.get_opt ()) > 0)
    switch (opt)
      {
	IMAGE_OUTPUT_OPTION_CASES (clp, dst_image_params);
	IMAGE_INPUT_OPTION_CASES (clp, src_image_params);
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
  src_image_params.file_name = clp.get_arg();
  ImageInput src_image (src_image_params);

  // We get the output image's size from the input image
  //
  unsigned width = src_image.width;
  unsigned height = src_image.height;

  // If the output image is going to be anti-aliased, it will consume
  // AA_FACTOR input pixels (in both vertical and horizontal directions)
  // for every output pixel produced; modify the output image size
  // accordingly.
  //
  if (dst_image_params.aa_factor > 1)
    {
      width /= dst_image_params.aa_factor;
      height /= dst_image_params.aa_factor;
    }

  // Open the output image using the resulting adjust size.
  //
  dst_image_params.file_name = clp.get_arg ();
  dst_image_params.width = width;
  dst_image_params.height = height;
  ImageOutput dst_image (dst_image_params);

  // Copy input image to output image, doing any processing
  //
  for (unsigned y = 0; y < src_image.height; y++)
    {
      // Get a row to write into from the output image
      //
      ImageRow &output_row = dst_image.next_row ();

      // Read into it from the input image
      //
      src_image.read_row (output_row);
    }
}

// arch-tag: 9852837a-ecf5-4400-9b79-f0cca96a6736
