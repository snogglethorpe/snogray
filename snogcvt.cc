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
#include "image.h"
#include "image-cmdline.h"

using namespace Snogray;
using namespace std;

int main (int argc, char *const *argv)
{
  // Command-line option specs
  static struct option long_options[] = {
    { "aa-factor",	required_argument, 0, 'a' },
    { "aa-overlap",	required_argument, 0, 'A' },
    { "aa-filter",	required_argument, 0, 'F' },
    { "gamma",		required_argument, 0, 'g' },
    { "input-format",	required_argument, 0, 'I' },
    { "output-format",	required_argument, 0, 'O' },
    { 0, 0, 0, 0 }
  };
  CmdLineParser clp (argc, argv, "a:A:F:g:O:", long_options);

  // Parameters set from the command line
  ImageCmdlineSourceParams src_image_params (clp);
  ImageCmdlineSinkParams dst_image_params (clp);

  // Parse command-line options
  int opt;
  while ((opt = clp.get_opt ()) > 0)
    switch (opt)
      {
	// Anti-aliasing options
      case 'a':
	dst_image_params.aa_factor = clp.unsigned_opt_arg ();
	break;
      case 'A':
	dst_image_params.aa_overlap = clp.unsigned_opt_arg ();
	break;
      case 'F':
	dst_image_params.parse_aa_filter_opt_arg ();
	break;

	// Output image options
      case 'g':
	dst_image_params.target_gamma = clp.float_opt_arg ();
	break;
      case 'O':
	dst_image_params.format = clp.opt_arg ();
	break;

	// Input image options
      case 'I':
	src_image_params.format = clp.opt_arg ();
	break;
      }

  if (clp.num_remaining_args() > 2)
    {
      cerr << "Usage: " << clp.prog_name()
	   << "[OPTION...] [OUTPUT_IMAGE_FILE]" << endl;
      cerr << "Try `" << clp.prog_name() << " --help' for more information"
	   << endl;
      exit (10);
    }
  src_image_params.file_name = clp.get_arg(); // 0 if none specified
  dst_image_params.file_name = clp.get_arg();	 // 0 if none specified

  ImageInput src_image (src_image_params);

  unsigned width = src_image.width;
  unsigned height = src_image.height;
  if (dst_image_params.aa_factor > 1)
    {
      width /= dst_image_params.aa_factor;
      height /= dst_image_params.aa_factor;
    }

  dst_image_params.width = width;
  dst_image_params.height = height;
  ImageOutput dst_image (dst_image_params);

  for (unsigned y = 0; y < src_image.height; y++)
    {
      ImageRow &output_row = dst_image.next_row ();
      src_image.read_row (output_row);
    }
}

// arch-tag: 9852837a-ecf5-4400-9b79-f0cca96a6736
