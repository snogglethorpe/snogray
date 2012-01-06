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

#define IMAGE_SCALED_OUTPUT_OPTIONS_HELP				\
  IMAGE_SAMPLED_OUTPUT_OPTIONS_HELP "\
\n\n\
      --preclamp             Clamp input to output range before filtering\n\
      --no-preclamp          Do not use preclamping\n\
			       (preclamping is used by default for\n\
				low-dynamic-range image output formats)"

#define IMAGE_SCALED_OUTPUT_SHORT_OPTIONS	\
  IMAGE_SAMPLED_OUTPUT_SHORT_OPTIONS

#define IMAGE_SCALED_OUTPUT_OPT_PRECLAMP (('p'<<24)+('r'<<16)+('c'<<8)+'l')
#define IMAGE_SCALED_OUTPUT_OPT_NO_PRECLAMP (('P'<<24)+('R'<<16)+('C'<<8)+'L')

#define IMAGE_SCALED_OUTPUT_LONG_OPTIONS				\
  IMAGE_SAMPLED_OUTPUT_LONG_OPTIONS,					\
  { "preclamp", no_argument, 0, IMAGE_SCALED_OUTPUT_OPT_PRECLAMP },	\
  { "no-preclamp", no_argument, 0, IMAGE_SCALED_OUTPUT_OPT_NO_PRECLAMP }

#define IMAGE_SCALED_OUTPUT_OPTION_CASES(clp, params)	\
  case IMAGE_SCALED_OUTPUT_OPT_PRECLAMP:		\
    params.set ("preclamp", true);			\
    break;						\
  case IMAGE_SCALED_OUTPUT_OPT_NO_PRECLAMP:		\
    params.set ("preclamp", false);			\
    break;						\
  IMAGE_SAMPLED_OUTPUT_OPTION_CASES (clp, params);


#endif // SNOGRAY_IMAGE_SCALED_OUTPUT_CMDLINE_H
