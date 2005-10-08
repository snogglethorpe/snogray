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


// Image transform functions

static inline float
adjust_component_contrast (Color::component_t com, float expon)
{
  com *= 2;
  com -= 1;
  if (com < 0)
    com = -pow (-com, expon);
  else
    com = pow (com, expon);
  return (com + 1) / 2;
}

// This might work alright for B&W, but handling each color component
// separately tends to screw up colors in odd ways.
//
static inline Color
adjust_contrast (const Color &col, float expon)
{
  return Color (adjust_component_contrast (col.red, expon),
		adjust_component_contrast (col.green, expon),
		adjust_component_contrast (col.blue, expon));
}

#if 0 /* this is just freaky */
static inline Color
adjust_contrast (const Color &col, float expon)
{
  Color::component_t r = col.red * 2 - 1;
  Color::component_t g = col.green * 2 - 1;
  Color::component_t b = col.blue * 2 - 1;

  if (col.intensity() > 0.5)
    return Color ((powf (r, expon) + 1) / 2,
		  (powf (g, expon) + 1) / 2,
		  (powf (b, expon) + 1) / 2);
  else
    return Color ((1 - powf (-r, expon)) / 2,
		  (1 - powf (-g, expon)) / 2,
		  (1 - powf (-b, expon)) / 2);
}
#endif



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
  os << "Change the format of or transform an image file" << endl
     << endl
<< " Transforms:" << endl
<< "  -e, --exposure=STOPS       Increase or decrease exposure by STOPS f-stops"
     << endl
<< "  -c, --contrast=POW         Increase or decrease contrast" << endl
     << endl
     << IMAGE_OUTPUT_OPTIONS_HELP << endl
     << endl
     << IMAGE_INPUT_OPTIONS_HELP << endl
     << endl
     << CMDLINEPARSER_GENERAL_OPTIONS_HELP << endl
     << endl
<< "If no filenames are given, standard input or output is used.  Input/output"
     << endl
<< "image formats are guessed using the corresponding filenames when possible"
     << endl
     << "(using the file's extension)." << endl;
}


int main (int argc, char *const *argv)
{
  // Command-line option specs
  //
  static struct option long_options[] = {
    { "exposure",	required_argument, 0, 'e' },
    { "contrast",	required_argument, 0, 'c' },
    IMAGE_INPUT_LONG_OPTIONS,
    IMAGE_OUTPUT_LONG_OPTIONS,
    CMDLINEPARSER_GENERAL_LONG_OPTIONS,
    { 0, 0, 0, 0 }
  };
  char short_options[] =
    "e:c:"
    IMAGE_OUTPUT_SHORT_OPTIONS
    IMAGE_INPUT_SHORT_OPTIONS
    CMDLINEPARSER_GENERAL_SHORT_OPTIONS;
  //
  CmdLineParser clp (argc, argv, short_options, long_options);

  // Parameters set from the command line
  //
  ImageCmdlineSourceParams src_image_params (clp);
  ImageCmdlineSinkParams dst_image_params (clp);

  float exposure = 0, contrast = 0;

  // Parse command-line options
  //
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

  float intensity_scale = (exposure == 0) ? 1 : powf (2.0, exposure);
  float intensity_expon = (contrast == 0) ? 1 : powf (2.0, -contrast);

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

      // If the user requested any transforms, do them to the row
      //
      if (contrast != 0 || exposure != 0)
	for (unsigned x = 0; x < output_row.width; x++)
	  {
	    Color col = output_row[x];
	    if (contrast != 0)
	      col = adjust_contrast (col, intensity_expon);
	    if (exposure != 0)
	      col *= intensity_scale;
	    output_row[x] = col;
	  }
    }
}

// arch-tag: 9852837a-ecf5-4400-9b79-f0cca96a6736
