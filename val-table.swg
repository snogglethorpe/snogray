// Lua table -> ValTable conversion
//
%{ #include "lua-funs.h" %}
%typemap(in, checkfn="lua_istable") const snogray::ValTable& (snogray::ValTable valtab)
%{	lua_pushvalue (L, $input);
	snogray::lua_store_into_val_table (L, valtab);
	$1 = &valtab; %}
%typecheck(SWIG_TYPECHECK_POINTER) const snogray::ValTable&
%{	$1 = lua_istable (L, $input); %}
