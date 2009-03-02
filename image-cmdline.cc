// image-cmdline.cc -- Support for command-line parsing of image parameters
//
//  Copyright (C) 2009  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <cctype>
#include <cstdlib>

#include "snogmath.h"
#include "val-table.h"
#include "cmdlineparser.h"

#include "image-cmdline.h"


using namespace snogray;

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
  // is treated as if it were preceded by "+".
  //
  char eop = *arg;
  if (isdigit (eop))
    {
      eop = '+';
      arg--;	// pretend we saw something
    }
  if (eop == '+' || eop == '-' || eop == '*' || eop == '/')
    {
      char *end;
      float val = strtod (++arg, &end);

      if (end != arg)
	{
	  if (eop == '+' || eop == '-')
	    val = pow (2., val);
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
