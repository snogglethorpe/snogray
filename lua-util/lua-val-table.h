// lua-val-table.h -- Functions for converting between ValTables and Lua tables
//
//  Copyright (C) 2010, 2011, 2013  Miles Bader <miles@gnu.org>
//
// This source code is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3, or (at
// your option) any later version.  See the file COPYING for more details.
//
// Written by Miles Bader <miles@gnu.org>
//

#ifndef SNOGRAY_LUA_VAL_TABLE_H
#define SNOGRAY_LUA_VAL_TABLE_H

#include "config.h"

extern "C"
{
#include "lua.h"
}


namespace snogray {

class ValTable;


// Copy all entries in VAL_TABLE into the Lua table on the top of the
// stack.  Returns nothing.
//
extern int lua_load_from_val_table (lua_State *L, const ValTable &val_table);

// Copy all entries from the Lua table on the top of the stack into
// VAL_TABLE.  Any entries with non-string keys are ignored (i.e., the
// table's array-part), as are any entries with values that cannot be
// stored into a VAL_TABLE.  Returns nothing.
//
extern int lua_store_into_val_table (lua_State *L, ValTable &val_table);


}

#endif // SNOGRAY_LUA_VAL_TABLE_H
