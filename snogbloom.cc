// snogbloom.cc -- Add glare effects ("bloom") to an image
//
//  Copyright (C) 2011, 2012  Miles Bader <miles@gnu.org>
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

#include "unique-ptr.h"
#include "cmdlineparser.h"
#include "image.h"
#include "image-input-cmdline.h"
#include "image-scaled-output-cmdline.h"
#include "glare.h"
#include "photopic-glare-psf.h"


using namespace snogray;
using namespace std;


// GaussianLimitPsf class

// A "modifier" point-spread-function that limits another PSF to a
// certain maximum angle, smoothly fading it towards the edge using a
// gaussian filter.
//
class GaussianLimitPsf : public GlarePsf
{
public:

  GaussianLimitPsf (const GlarePsf *_psf, float limit_angle)
    : psf (_psf), limit (limit_angle),
      sigma (5.f / (limit * limit)),
      gauss_edge_value (gauss (limit)),
      filter_scale (1 / (1 - gauss_edge_value))
  { }

  // Return the value of the PSF at an angle of THETA radians from the
  // central axis.
  //
  virtual float operator() (float theta) const
  {
    if (theta > limit)
      return 0;
    else
      return (*psf) (theta) * filter (theta);
  }

private:

  UniquePtr<const GlarePsf> psf;

  // Return the value of our gaussian function at THETA.  This is the
  // "raw" gaussian, before we've adjusted to compensate for the edge
  // value.
  //
  float gauss (float theta) const
  {
    return exp (-sigma*theta*theta);
  }

  // Return the value of our filter at THETA.  This is the "real"
  // value, with a value of exactly 1 at zero, and exactly 0 at LIMIT.
  //
  float filter (float theta) const
  {
    return (gauss (theta) - gauss_edge_value) * filter_scale;
  }

  // The limit we are restricting.
  //
  float limit;

  // The value σ for the gaussian filter.  This is calculated so that
  // it approaches zero at the limit.
  //
  float sigma;

  // Value of the gaussian filter at the limit.  We subtract this from
  // the calculated filter value to ensure that it exactly hits zero
  // at the limit.
  //
  float gauss_edge_value;

  // A scale factor to compensate for the fact that we subtract
  // FILTER_EDGE_VALUE from the filter.  A gaussian filter has a value
  // of 1 at the center, so this is 1 / (1 - GAUSS_EDGE_VALUE).
  //
  float filter_scale;
};


// command-line help

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

  os << "Add glare effects (\"bloom\") to an image"
n
s "  -f, --field-of-view=DEG    Image field-of-view in degrees (default 46.8)"
s "  -l, --limit-angle=ANGLE    Limit glare function to ANGLE degrees"
s "  -g, --glare-only           Output only the computed glare"
s "      --threshold=INTENS     Add glare for intensities above INTENS (default 1)"
//s "                               (by default based on output image format)"
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
s "Note that an alpha channel in the input image is ignored."
n
    ;

#undef s
#undef n
}


// snogbloom main

#define OPT_THRESHOLD 5

int main (int argc, char *const *argv)
{
  // Command-line option specs
  //
  static struct option long_options[] = {
    { "field-of-view", required_argument, 0, 'f' },
    { "limit-angle", required_argument, 0, 'l' },
    { "glare-only", no_argument, 0, 'g' },
    { "threshold", required_argument, 0, OPT_THRESHOLD },
    IMAGE_INPUT_LONG_OPTIONS,
    IMAGE_SCALED_OUTPUT_LONG_OPTIONS,
    CMDLINEPARSER_GENERAL_LONG_OPTIONS,
    { 0, 0, 0, 0 }
  };
  char short_options[] =
    "f:l:g"
    IMAGE_INPUT_SHORT_OPTIONS
    IMAGE_SCALED_OUTPUT_SHORT_OPTIONS
    CMDLINEPARSER_GENERAL_SHORT_OPTIONS;
  //
  CmdLineParser clp (argc, argv, short_options, long_options);
  float diag_field_of_view = 46.8f * PIf / 180;
  bool glare_only = false;
  float threshold = 1;
  float limit_angle = 0;

  // Parameters set from the command line
  //
  ValTable src_params, dst_params;

  // Parse command-line options
  //
  int opt;
  while ((opt = clp.get_opt ()) > 0)
    switch (opt)
      {
      case 'f':
	diag_field_of_view = clp.float_opt_arg () * PIf / 180;
	break;
      case 'l':
	limit_angle = clp.float_opt_arg () * PIf / 180;
	break;
      case 'g':
	glare_only = true;
	break;
      case OPT_THRESHOLD:
	threshold = clp.float_opt_arg ();
	break;

	IMAGE_INPUT_OPTION_CASES (clp, src_params);
	IMAGE_SCALED_OUTPUT_OPTION_CASES (clp, dst_params);
	CMDLINEPARSER_GENERAL_OPTION_CASES (clp);
      }

  if (clp.num_remaining_args() > 2)
    {
      usage (clp, cerr);
      clp.try_help_err ();
    }

  // Load the input image.
  //
  Image image (clp.get_arg(), src_params);

  UniquePtr<const GlarePsf> glare_psf (new PhotopicGlarePsf ());

  if (limit_angle)
    glare_psf.reset (new GaussianLimitPsf (glare_psf.release (), limit_angle));

  // Apply the bloom filter.
  //
  add_glare (*glare_psf, image, diag_field_of_view, threshold, glare_only);

  // Save it to the output file.
  //
  image.save (clp.get_arg(), dst_params);
}
