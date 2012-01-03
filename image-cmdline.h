// image-cmdline.h -- Support for command-line parsing of image parameters
//
//  Copyright (C) 2005-2007, 2009-2012  Miles Bader <miles@gnu.org>
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
    clp.parse_opt_arg (params);			\
    break;


// Image output options

// Note that the macros named "IMAGE_OUTPUT_CVTONLY_..." are for
// options that really only make sense for snogcvt and similar
// utilities.  In particular the notion of "preclamping" doesn't work
// in the case of rendering output, where samples from rendering have
// no real spatial coherence (it would work better if there were an
// intermediate accumulation stage before final image output).


#define IMAGE_OUTPUT_OPTIONS_HELP "\
  -s, --size=WIDTHxHEIGHT    Set image size to WIDTH x HEIGHT pixels/lines\n\
  -s, --size=SIZE            Set largest image dimension to SIZE,\n\
                             preserving aspect ratio\n\
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
                               OPT1=VAL1; current options include:\n\
                                 \"format\"  -- output file type\n\
                                 \"gamma\"   -- target gamma correction\n\
                                 \"quality\" -- image compression quality (0-100)\n\
                                 \"filter\"  -- output filter\n\
                                 \"exposure\"-- output exposure"
#define IMAGE_OUTPUT_CVTONLY_OPTIONS_HELP "\
  -p, --preclamp             Clamp input to output range before filtering\n\
                                (this can yield better anti-aliasing when\n\
                                 downsampling from an HDR input image to\n\
                                 a smaller LDR output image)"

#define IMAGE_OUTPUT_SHORT_OPTIONS "s:e:F:O:"
#define IMAGE_OUTPUT_CVTONLY_SHORT_OPTIONS "p"

#define IMAGE_OUTPUT_LONG_OPTIONS			\
  { "size",		required_argument, 0, 's' },	\
  { "filter",		required_argument, 0, 'F' },	\
  { "exposure",		required_argument, 0, 'e' },	\
  { "output-options",	required_argument, 0, 'O' }
#define IMAGE_OUTPUT_CVTONLY_LONG_OPTIONS		\
  { "preclamp",		no_argument,	   0, 'p' }	\

#define IMAGE_OUTPUT_OPTION_CASES(clp, params)				\
  case 'F':								\
    clp.store_opt_arg_with_sub_options ("filter", params, ".", "/,");	\
    break;								\
  case 's':								\
    parse_image_size_option (clp, params);				\
    break;								\
  case 'e':								\
    parse_image_exposure_option (clp, params);				\
    break;								\
  case 'O':								\
    clp.parse_opt_arg (params);						\
    break;
#define IMAGE_OUTPUT_CVTONLY_OPTION_CASES(clp, params)			\
  case 'p':								\
    params.set ("preclamp", true);					\
    break;


namespace snogray {

class ValTable;
class CmdLineParser;

// Parse a size option argument.  If both a width and height are
// specified, "width" and "height" entries are added to params.  If
// only a single number is specified, a "size" entry is added instead.
//
extern void parse_image_size_option (CmdLineParser &clp, ValTable &params);

// Parse the argument of a command-line exposure option, storing the
// resulting parameters into PARAMS.  Parameters possibly affected are
// "exposure" and "contrast".
//
extern void parse_image_exposure_option (CmdLineParser &clp, ValTable &params);

// Return the width/height specified by PARAMS in WIDTH and HEIGHT.
//
// If PARAMS contains "width" and "height" parameters (it should
// contain either both or neither), they are returned directly.
// Otherwise, if PARAMS contains "size" parameter, it is used to set
// the largest dimension, and the other dimension calculated using
// ASPECT_RATIO; if there is no "size" parameter, DEFAULT_SIZE is used
// instead.
//
extern void get_image_size (const ValTable &params,
			    float aspect_ratio, unsigned default_size,
			    unsigned &width, unsigned &height);


}


#endif /* SNOGRAY_IMAGE_CMDLINE_H */


// arch-tag: d728801d-ce3a-414e-89a1-60b259197526
