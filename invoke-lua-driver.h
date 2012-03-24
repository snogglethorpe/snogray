// invoke-driver.h -- Find and invoke the top-level Lua driver
//
//  Copyright (C) 2011, 2012  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_INVOKE_DRIVER_H
#define SNOGRAY_INVOKE_DRIVER_H

#include <string>


namespace snogray {


// Locate the Lua driver file called DRIVER_NAME, and invoke it with a
// Lua table containing the strings in ARGV, which should be arguments
// the process was invoked with.
//
// If an error occurs, print an error message to std::cerr and exit.
//
// ARGV[0] is assumed to be the command name, which is used both for
// error messages, and as a possible location to search for related
// files.  DRIVER_NAME will be searched for in "appropriate places":
// standard installation locations, and relative to the command (for
// running an un-installed binary).
//
extern void invoke_lua_driver (const std::string &driver_name,
			       const char **argv);


}

#endif // SNOGRAY_INVOKE_DRIVER_H
