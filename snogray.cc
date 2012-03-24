// snogray.cc -- Main driver for snogray ray tracer
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

#include <string>
#include <cstring>

#include "invoke-lua-driver.h"

int main (int, const char **argv)
{
  std::string driver = "snogray.lua";

  // An initial magic "--driver DRIVER_NAME" option replaces the name
  // of the driver, and then invokes it with the option removed.
  // [It's "magic" because it can only occur as the first option, and
  // must be in exactly that form.]
  //
  if (argv[1] && strcmp (argv[1], "--driver") == 0)
    {
      driver = argv[2];
      argv[2] = argv[0];
      argv += 2;
    }

  snogray::invoke_lua_driver (driver, argv);

  return 0;
}
