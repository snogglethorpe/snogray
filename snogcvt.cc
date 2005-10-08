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
#include <cmath>

#include "cmdlineparser.h"
#include "image.h"
#include "image-cmdline.h"

using namespace Snogray;
using namespace std;

static void
usage (CmdLineParser &clp, ostream &os)
{
  os << "Usage: " << clp.prog_name()
     << "[OPTION...] [INPUT_IMAGE_FILE [OUTPUT_IMAGE_FILE]]" << endl;
}

static void
help (CmdLineParser &clp, ostream &os)
{
  usage (clp, os);
  os << "\
Change the format of or transform an image file\n\
\n\
 Transforms:\n\
  -e, --exposure=STOPS       Increase or decrease exposure by STOPS f-stops\n\
  -c, --contrast=POW         Increase or decrease contrast\n\
\n\
 Output options:\n\
  -g, --gamma=GAMMA          Do gamma correction for a target display\n\
                             gamma of GAMMA (default: 2.2, for output\n\
                             formats that need gamma-correction)\n\
  -O, --output-format=FMT    Output image format FMT (one of: exr, png)\n\
\n\
 Input options:\n\
  -I, --input-format=FMT     Input image format FMT (one of: exr, png)\n\
\n\
 Anti-aliasing:\n\
  -a, --aa-factor=N          Use NxN input pixels to compute each output pixel\n\
  -A, --aa-overlap=M         Include M adjacent input pixels in anti-aliasing\n\
  -F, --aa-filter=NAME       Use anti-aliasing filter NAME (one of: box,\n\
                             triang, gauss; default: gauss)\n\
\n\
      --help                 Output this help message\n\
      --version              Output program version\n\
\n\
If no filenames are given, standard input or output is used.  Input/output\n\
image formats are guessed using the corresponding filenames when possible\n\
(using the file's extension).\n\
";
}

#define OPT_HELP	1
#define OPT_VERSION	2

int main (int argc, char *const *argv)
{
  // Command-line option specs
  static struct option long_options[] = {
    { "exposure",	required_argument, 0, 'e' },
    { "contrast",	required_argument, 0, 'c' },
    { "aa-factor",	required_argument, 0, 'a' },
    { "aa-overlap",	required_argument, 0, 'A' },
    { "aa-filter",	required_argument, 0, 'F' },
    { "gamma",		required_argument, 0, 'g' },
    { "input-format",	required_argument, 0, 'I' },
    { "output-format",	required_argument, 0, 'O' },
    { "help",		no_argument,	   0, OPT_HELP },
    { "version",	no_argument,	   0, OPT_VERSION },
    { 0, 0, 0, 0 }
  };
  CmdLineParser clp (argc, argv, "e:c:a:A:F:g:O:", long_options);

  // Parameters set from the command line
  ImageCmdlineSourceParams src_image_params (clp);
  ImageCmdlineSinkParams dst_image_params (clp);

  float exposure = 0, contrast = 1;

  // Parse command-line options
  int opt;
  while ((opt = clp.get_opt ()) > 0)
    switch (opt)
      {
      case 'e':
	exposure = clp.float_opt_arg ();
	break;
      case 'c':
	contrast = clp.float_opt_arg ();
	break;

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

      case OPT_HELP:
	help (clp, cout);
	exit (0);
      case OPT_VERSION:
	cout << "snogcvt (snogray) 1.0" << endl;
	exit (0);
      }

  if (clp.num_remaining_args() > 2)
    {
      usage (clp, cerr);
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

  float intensity_scale = (exposure == 0) ? 1 : powf (2.0, exposure);

  for (unsigned y = 0; y < src_image.height; y++)
    {
      ImageRow &output_row = dst_image.next_row ();

      src_image.read_row (output_row);

      if (contrast != 1 || exposure != 0)
	for (unsigned x = 0; x < src_image.width; x++)
	  {
	    Color col = output_row[x];
	    if (contrast != 1)
	      col = col.pow (contrast);
	    if (exposure != 0)
	      col *= intensity_scale;
	    output_row[x] = col;
	  }
    }
}

// arch-tag: 9852837a-ecf5-4400-9b79-f0cca96a6736
