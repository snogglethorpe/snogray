// snogpaths.cc -- Handling of installation paths
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

#include "snogpaths-data.h"

#include "snogpaths.h"


//
// Currently these functions just return the statically configured
// install paths.
//
// A better method would be try and make everything "relocatable", by
// detecting the run-time install prefix, and making other names
// relative to that.  This can be done as follows:
//
// 1. Determine the absolute runtime name of the executable.
//    There should be a function that main calls to pass in
//    argv[0]; it can be used as-is if absolute, or searched for
//    in PATH if not.
// 
// 2. Determine the relative bindir suffix by removing the static
//    install prefix from the static bindir (if the static bindir
//    is not relative to the static prefix, give up).
//
// 3. Remove the filename and relative bindir suffix from the end
//    of the absolute runtime executable name (if they don't
//    match, give up), yielding the runtime prefix.
//
// 4. Determine the relative suffixes for other paths (datadir,
//    etc) by removing the static prefix from their static
//    versions, as in (2).
//
// 5. Append the runtime prefix and the relative suffixes
//    determined in (4) to yield the absolute runtime paths.
//

std::string
snogray::installed_prefix ()
{
  return SNOGRAY_PREFIX;
}

std::string
snogray::installed_bindir ()
{
  return SNOGRAY_BINDIR;
}

std::string
snogray::installed_pkgdatadir ()
{
  return SNOGRAY_PKGDATADIR;
}
