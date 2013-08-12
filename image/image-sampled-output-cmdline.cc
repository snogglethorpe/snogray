// image-sampled-output-cmdline.cc -- Support for command-line parsing
//	of output image parameters
//
//  Copyright (C) 2009, 2010, 2012, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cctype>
#include <cstring>
#include <cstdlib>

#include "util/snogmath.h"
#include "util/val-table.h"
#include "cli/cmdlineparser.h"

#include "image-sampled-output-cmdline.h"


using namespace snogray;


// parse_image_size_option

// Parse a size option argument.  If both a width and height are
// specified, "width" and "height" entries are added to params.  If
// only a single number is specified, a "size" entry is added instead.
//
void
snogray::parse_image_size_option (CmdLineParser &clp, ValTable &params)
{
  const char *arg = clp.opt_arg ();
  char *end = 0;

  unsigned num = strtoul (arg, &end, 10);

  if (end && end != arg)
    {
      // If no height is given, it will be set according to the camera's
      // aspect ratio
      //
      if (*end == '\0') 
	{
	  params.set ("size", num);
	  return;
	}

      arg = end + strspn (end, " ,x");

      unsigned height = strtoul (arg, &end, 10);

      params.set ("width", num);
      params.set ("height", height);

      if (end && end != arg && *end == '\0')
	return;
    }

  clp.opt_err ("requires a size specification (WIDTHxHEIGHT, or SIZE)");
}


// parse_image_exposure_option

// Parse the argument of a command-line exposure option, storing the
// resulting parameters into PARAMS.  Parameters possibly affected are
// "exposure" and "contrast".
//
void
snogray::parse_image_exposure_option (CmdLineParser &clp, ValTable &params)
{
  const char *arg = clp.opt_arg ();
  const char *arg_beg = arg;

  bool ok = true;		// error flag

  // First look for an exposure; it can either an explicit multiplicative
  // factor, prefixed by "*" or "/", or an adjustment in "stops", prefixed
  // by "+" or "-" (+N is equivalent to *(2^N)).  A number with no prefix
  // is treated as if it were preceded by "*".
  //
  char eop = *arg;
  if (isdigit (eop))
    {
      eop = '*';
      arg--;	// pretend we saw something
    }
  if (eop == '+' || eop == '-' || eop == '*' || eop == '/')
    {
      char *end;
      float val = strtod (++arg, &end);

      if (end != arg)
	{
	  if (eop == '+' || eop == '-')
	    val = pow (2.f, val);
	  if (eop == '/' || eop == '-')
	    val = 1 / val;

	  params.set ("exposure", val);
	}
      else
	ok = false;

      arg = end;
    }

  // Now look for a contrast adjustment, which should be prefixed by "^".
  //
  if (*arg == '^')
    {
      char *end;
      float val = strtod (++arg, &end);

      if (end != arg)
	params.set ("contrast", val);
      else
	ok = false;

      arg = end;
    }

  if (!ok || *arg != '\0')
    clp.opt_err ("argument has invalid syntax (expected (+|-|*|/)NUM[^NUM])");
  else if (arg == arg_beg)
    clp.opt_err ("requires an argument");
}


// get_image_size

// Return the width/height specified by PARAMS in WIDTH and HEIGHT.
//
// If PARAMS contains "width" and "height" parameters (it should
// contain either both or neither), they are returned directly.
// Otherwise, if PARAMS contains "size" parameter, it is used to set
// the largest dimension, and the other dimension calculated using
// ASPECT_RATIO; if there is no "size" parameter, DEFAULT_SIZE is used
// instead.
//
void
snogray::get_image_size (const ValTable &params,
			 float aspect_ratio, unsigned default_size,
			 unsigned &width, unsigned &height)
{
  width = params.get_uint ("width", 0);
  height = params.get_uint ("height", 0);

  if (!width || !height)
    // Otherwise, the image size was not fully specified, so use a
    // "size" and ASPECT_RATIO to set them.  SIZE is either a "size"
    // parameter from PARAMS, or DEFAULT_SIZE.
    {
      unsigned size = params.get_uint ("size", default_size);
      width = (aspect_ratio >= 1) ? size : unsigned (size * aspect_ratio);
      height = (aspect_ratio >= 1) ? unsigned (size / aspect_ratio) : size;
    }
}
