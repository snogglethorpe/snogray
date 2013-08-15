// lua-setup.h -- Create a new snogray-specific lua state
//
//  Copyright (C) 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_LUA_SETUP_H
#define SNOGRAY_LUA_SETUP_H

extern "C"
{
#include "lua.h"
}


namespace snogray {


// Return a new Lua state setup with our special environment.
// UNINSTALLED_DIR is a directory to search for lua source files in
// uninstalled-mode; if it is empty uninstalled-mode isn't used.
//
extern lua_State *new_snogray_lua_state (
		    const std::string &uninstalled_dir = "");


}

#endif // SNOGRAY_LUA_SETUP_H
