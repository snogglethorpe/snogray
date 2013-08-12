// lua-setup.h -- Create a new snogray-specific lua state
//
//  Copyright (C) 2011  Miles Bader <miles@gnu.org>
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
//
extern lua_State *new_snogray_lua_state ();


}

#endif // SNOGRAY_LUA_SETUP_H
