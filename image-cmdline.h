// image-cmdline.h -- Support for command-line parsing of image parameters
//
//  Copyright (C) 2005, 2006, 2007, 2009, 2010  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef __IMAGE_CMDLINE_H__
#define __IMAGE_CMDLINE_H__

// The following macros can be used in defining option parsers.

// Image input options

#define IMAGE_INPUT_OPTIONS_HELP "\
  -I, --input-options=OPTS   Set input-image options; OPTS has the format\n\
                               OPT1=VAL1[,...]; current options include:\n\
                                 \"format\" -- set the type of input file"

#define IMAGE_INPUT_SHORT_OPTIONS "I:"

#define IMAGE_INPUT_LONG_OPTIONS			\
 { "input-options",	required_argument, 0, 'I' }

#define IMAGE_INPUT_OPTION_CASES(clp, params)	\
  case 'I':					\
    params.parse (clp.opt_arg ());		\
    break;


// Image output options

#define IMAGE_OUTPUT_OPTIONS_HELP "\
  -e, --exposure=EXPOSURE    Increase/decrease output brightness/contrast\n\
                               EXPOSURE can have one of the forms:\n\
                                 +STOPS  -- Make output 2^STOPS times brighter\n\
                                 -STOPS  -- Make output 2^STOPS times dimmer\n\
                                 *SCALE  -- Make output SCALE times brighter\n\
                                 /SCALE  -- Make output SCALE times dimmer\n\
                                 ^POWER  -- Raise output to the POWER power\n\
  -F, --filter=FILTER[/PARAM=VAL...]\n\
                             Filter to apply to the output image, and\n\
                               optional parameters; FILTER may be one of\n\
                               \"mitchell\", \"gauss\", or \"box\"\n\
                               (default \"mitchell\")\n\
\n\
  -O, --output-options=OPTS  Set output-image options; OPTS has the format\n\
                               OPT1=VAL1[,...]; current options include:\n\
                                 \"format\"  -- output file type\n\
                                 \"gamma\"   -- target gamma correction\n\
                                 \"quality\" -- image compression quality (0-100)\n\
                                 \"filter\"  -- output filter\n\
                                 \"exposure\"-- output exposure"

#define IMAGE_OUTPUT_SHORT_OPTIONS "e:F:O:"

#define IMAGE_OUTPUT_LONG_OPTIONS			\
  { "filter",		required_argument, 0, 'F' },	\
  { "exposure",		required_argument, 0, 'e' },	\
  { "output-options",	required_argument, 0, 'O' }

#define IMAGE_OUTPUT_OPTION_CASES(clp, params)		\
  case 'F':						\
    params.set ("filter", clp.opt_arg ());		\
    break;						\
  case 'e':						\
    parse_image_exposure_option (clp, params);		\
    break;						\
  case 'O':						\
    params.parse (clp.opt_arg ());			\
    break;


namespace snogray {

class ValTable;
class CmdLineParser;

// Parse the argument of a command-line exposure option, storing the
// resulting parameters into PARAMS.  Parameters possibly affected are
// "exposure" and "contrast".
//
void parse_image_exposure_option (CmdLineParser &clp, ValTable &params);

}


#endif /* __IMAGE_CMDLINE_H__ */


// arch-tag: d728801d-ce3a-414e-89a1-60b259197526
