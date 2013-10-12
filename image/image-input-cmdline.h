// image-input-cmdline.h -- Support for command-line parsing of
//	input image parameters
//
//  Copyright (C) 2005-2007, 2009-2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_IMAGE_CMDLINE_H
#define SNOGRAY_IMAGE_CMDLINE_H

#include "cli/cmdlineparser.h"


#define IMAGE_INPUT_OPTIONS_HELP "\
  -I, --input-options=OPTS   Set input-image options; OPTS has the format\n\
                               OPT1=VAL1[,...]; current options include:\n\
                                 \"format\" -- set the type of input file"

#define IMAGE_INPUT_SHORT_OPTIONS "I:"

#define IMAGE_INPUT_LONG_OPTIONS			\
 { "input-options",	required_argument, 0, 'I' }

#define IMAGE_INPUT_OPTION_CASES(clp, params)	\
  case 'I':					\
    clp.parse_opt_arg (params);			\
    break;


#endif // SNOGRAY_IMAGE_CMDLINE_H
