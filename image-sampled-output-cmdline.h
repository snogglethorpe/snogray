// image-sampled-output-cmdline.h -- Support for command-line parsing
//	of output image parameters
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

#ifndef SNOGRAY_IMAGE_SAMPLED_OUTPUT_CMDLINE_H
#define SNOGRAY_IMAGE_SAMPLED_OUTPUT_CMDLINE_H

// The following macros can be used in defining output option parsers.


#define IMAGE_SAMPLED_OUTPUT_OPTIONS_HELP "\
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
      --no-dither            Do not add dithering noise to LDR output formats\n\
                               (dithering is used by default for low-dynamic-\n\
                                range output formats, where it helps prevent\n\
                                banding of very shallow gradients)\n\
\n\
  -O, --output-options=OPTS  Set output-image options; OPTS has the format\n\
                               OPT1=VAL1; current options include:\n\
                                 \"format\"  -- output file type\n\
                                 \"gamma\"   -- target gamma correction\n\
                                 \"quality\" -- image compression quality (0-100)\n\
                                 \"filter\"  -- output filter\n\
                                 \"exposure\"-- output exposure"

#define IMAGE_SAMPLED_OUTPUT_SHORT_OPTIONS "s:e:F:O:"

#define IMAGE_SAMPLED_OUTPUT_OPT_DITHER (('d'<<24)+('t'<<16)+('h'<<8)+'r')
#define IMAGE_SAMPLED_OUTPUT_OPT_NO_DITHER (('D'<<24)+('T'<<16)+('H'<<8)+'R')

#define IMAGE_SAMPLED_OUTPUT_LONG_OPTIONS				\
  { "size",		required_argument, 0, 's' },			\
  { "filter",		required_argument, 0, 'F' },			\
  { "exposure",		required_argument, 0, 'e' },			\
  { "dither",		no_argument,	   0, IMAGE_SAMPLED_OUTPUT_OPT_DITHER }, \
  { "no-dither",	no_argument,	   0, IMAGE_SAMPLED_OUTPUT_OPT_NO_DITHER }, \
  { "output-options",	required_argument, 0, 'O' }

#define IMAGE_SAMPLED_OUTPUT_OPTION_CASES(clp, params)			\
  case 'F':								\
    clp.store_opt_arg_with_sub_options ("filter", params, ".", "/,");	\
    break;								\
  case 's':								\
    parse_image_size_option (clp, params);				\
    break;								\
  case 'e':								\
    parse_image_exposure_option (clp, params);				\
    break;								\
  case IMAGE_SAMPLED_OUTPUT_OPT_DITHER:					\
    params.set ("dither", true);					\
    break;								\
  case IMAGE_SAMPLED_OUTPUT_OPT_NO_DITHER:				\
    params.set ("dither", false);					\
    break;								\
  case 'O':								\
    clp.parse_opt_arg (params);						\
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


#endif // SNOGRAY_IMAGE_SAMPLED_OUTPUT_CMDLINE_H
