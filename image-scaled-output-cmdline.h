// image-scaled-output-cmdline.h -- Support for command-line parsing
//	of scaled output image parameters
//
//  Copyright (C) 2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_IMAGE_SCALED_OUTPUT_CMDLINE_H
#define SNOGRAY_IMAGE_SCALED_OUTPUT_CMDLINE_H

#include "image-sampled-output-cmdline.h"


// The following macros can be used in defining output option parsers
// for use with the ImageScaledOutput class.

#define IMAGE_SCALED_OUTPUT_OPTIONS_HELP "\
  -p, --preclamp             Clamp input to output range before filtering\n\
                               (this can yield better anti-aliasing when\n\
                                downsampling from an HDR input image to\n\
                                a smaller LDR output image)\n"		\
  IMAGE_SAMPLED_OUTPUT_OPTIONS_HELP

#define IMAGE_SCALED_OUTPUT_SHORT_OPTIONS	\
  "p"						\
  IMAGE_SAMPLED_OUTPUT_SHORT_OPTIONS

#define IMAGE_SCALED_OUTPUT_LONG_OPTIONS	\
  { "preclamp", no_argument, 0, 'p' },		\
  IMAGE_SAMPLED_OUTPUT_LONG_OPTIONS

#define IMAGE_SCALED_OUTPUT_OPTION_CASES(clp, params)	\
  case 'p':						\
    params.set ("preclamp", true);			\
    break;						\
  IMAGE_SAMPLED_OUTPUT_OPTION_CASES (clp, params);


#endif // SNOGRAY_IMAGE_SCALED_OUTPUT_CMDLINE_H
