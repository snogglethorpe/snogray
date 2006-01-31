// image-cmdline.h -- Support for command-line parsing of image parameters
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_CMDLINE_H__
#define __IMAGE_CMDLINE_H__

#include "image-io.h"
#include "cmdlineparser.h"

// The following macros can be used in defining option parsers.

// Image input options
//
#define IMAGE_INPUT_OPTIONS_HELP "\
 Input options:\n\
  -I, --input-format=FMT     Input image format FMT\n\
                               (one of: exr, png, ppm, pfm, jpeg, hdr, pic)"
//
#define IMAGE_INPUT_SHORT_OPTIONS "I:"
//
#define IMAGE_INPUT_LONG_OPTIONS			\
 { "input-format",	required_argument, 0, 'I' }
//
#define IMAGE_INPUT_OPTION_CASES(clp, params)	\
  case 'I':					\
    params.format = clp.opt_arg ();		\
    break;

// Image output options
//
#define IMAGE_OUTPUT_OPTIONS_HELP "\
 Output options:\n\
  -O, --output-format=FMT    Output image format FMT\n\
                               (one of: exr, png, ppm, pfm, jpeg, hdr, pic)\n\
  -g, --gamma=GAMMA          Do gamma correction for a target display\n\
                               gamma of GAMMA (default: 2.2, for output\n\
                               formats that need gamma-correction)\n\
  -Q, --quality=PERCENT	     Set output quality, for formats that support it\n\
                               (range: 0-100; default 98)\n\
\n\
 Anti-aliasing:\n\
  -a, --aa-factor=N          Use NxN input pixels to compute each output pixel\n\
  -A, --aa-overlap=M         Include M adjacent input pixels in anti-aliasing\n\
  -F, --aa-filter=NAME       Use anti-aliasing filter NAME (one of: box,\n\
                               triang, gauss; default: gauss)\n\
\n\
 Transforms:\n\
  -e, --exposure=STOPS       Increase or decrease exposure by STOPS f-stops"
//
#define IMAGE_OUTPUT_SHORT_OPTIONS "a:A:F:O:g:Q:e:"
//
#define IMAGE_OUTPUT_LONG_OPTIONS			\
  { "output-format",	required_argument, 0, 'O' },	\
  { "gamma",		required_argument, 0, 'g' },	\
  { "quality",		required_argument, 0, 'Q' },	\
  { "aa-factor",	required_argument, 0, 'a' },	\
  { "aa-overlap",	required_argument, 0, 'A' },	\
  { "aa-filter",	required_argument, 0, 'F' },	\
  { "exposure",		required_argument, 0, 'e' }
//
#define IMAGE_OUTPUT_OPTION_CASES(clp, params)		\
  case 'O':						\
    params.format = clp.opt_arg ();			\
    break;						\
  case 'g':						\
    params.target_gamma = clp.float_opt_arg ();		\
    break;						\
  case 'Q':						\
    params.quality = clp.float_opt_arg ();		\
    break;						\
  /* Anti-aliasing options */				\
  case 'a':						\
    params.aa_factor = clp.unsigned_opt_arg ();		\
    break;						\
  case 'A':						\
    params.aa_overlap = clp.unsigned_opt_arg ();	\
    break;						\
  case 'F':						\
    params.parse_aa_filter_opt_arg ();			\
    break;						\
  /* Transform options */				\
  case 'e':						\
    params.exposure = clp.float_opt_arg ();		\
    break;



namespace Snogray {

// This class can be used when parsing image parameters
struct ImageCmdlineSinkParams : ImageSinkParams
{
  ImageCmdlineSinkParams (CmdLineParser &_clp) : clp (_clp) { }

  // This is called when something wrong is detect with some parameter
  virtual void error (const std::string &msg) const;

  void parse_aa_filter_opt_arg ()
  {
    const char *filt_name = clp.opt_arg ();
    if (strcmp (filt_name, "box") == 0)
      aa_filter = ImageOutput::aa_box_filter;
    else if (strcmp (filt_name, "triang") == 0)
      aa_filter = ImageOutput::aa_triang_filter;
    else if (strcmp (filt_name, "gauss") == 0)
      aa_filter = ImageOutput::aa_gauss_filter;
    else
      clp.opt_err ("requires an anti-aliasing filter name"
		   " (box, triang, gauss)");
  }

  // Returns a name for the specified aa_filter
  const char *aa_filter_name()
  {
    float (*filt) (int offs, unsigned size)
      = aa_filter ? aa_filter : ImageOutput::DEFAULT_AA_FILTER;
    if (filt == ImageOutput::aa_box_filter)
      return "box";
    else if (filt == ImageOutput::aa_triang_filter)
      return "triang";
    else if (filt == ImageOutput::aa_gauss_filter)
      return "gauss";
    else
      return "???";
  }

  // We keep track of this so that we may format error messages nicely
  CmdLineParser &clp;
};

// This class can be used when parsing image parameters
struct ImageCmdlineSourceParams : ImageSourceParams
{
  ImageCmdlineSourceParams (CmdLineParser &_clp) : clp (_clp) { }

  // This is called when something wrong is detect with some parameter
  virtual void error (const std::string &msg) const;

  // We keep track of this so that we may format error messages nicely
  CmdLineParser &clp;
};

}

#endif /* __IMAGE_CMDLINE_H__ */

// arch-tag: d728801d-ce3a-414e-89a1-60b259197526
