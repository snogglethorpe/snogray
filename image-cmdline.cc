// image-cmdline.cc -- Support for command-line parsing of image parameters
//
//  Copyright (C) 2005  Miles Bader <miles@gnu.org>
//
// This file is subject to the terms and conditions of the GNU General
// Public License.  See the file COPYING in the main directory of this
// archive for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#include <iostream>

#include "image-cmdline.h"

using namespace Snogray;
using namespace std;

static void
fatal_err (CmdLineParser &clp, const char *msg,
	   const char *name, const char *default_name)
{
  if (! name)
    name = default_name;
  cerr << clp.err_pfx() << name << ": " << msg << endl;
  exit (25);
}

void
ImageCmdlineSinkParams::error (const char *msg) const
{
  fatal_err (clp, msg, file_name, "<standard output>");
}

void
ImageCmdlineSourceParams::error (const char *msg) const
{
  fatal_err (clp, msg, file_name, "<standard input>");
}

// arch-tag: b16c60de-3181-4c5c-8848-913d61217333
