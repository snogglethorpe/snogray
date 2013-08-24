# val-table.swg -- Lua table -> ValTable conversion
#
#  Copyright (C) 2011, 2013  Miles Bader <miles@gnu.org>
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 3, or (at
# your option) any later version.  See the file COPYING for more details.
#
# Written by Miles Bader <miles@gnu.org>
#

// Lua table -> ValTable conversion
//
%{ #include "lua-util/lua-val-table.h" %}
%typemap(in, checkfn="lua_istable") const snogray::ValTable& (snogray::ValTable valtab)
%{	lua_pushvalue (L, $input);
	snogray::lua_store_into_val_table (L, valtab);
	$1 = &valtab; %}
%typecheck(SWIG_TYPECHECK_POINTER) const snogray::ValTable&
%{	$1 = lua_istable (L, $input); %}
