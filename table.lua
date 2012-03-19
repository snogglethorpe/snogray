-- table.lua -- Miscellaneous table functions
--
--  Copyright (C) 2012  Miles Bader <miles@gnu.org>
--
-- This source code is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License as
-- published by the Free Software Foundation; either version 3, or (at
-- your option) any later version.  See the file COPYING for more details.
--
-- Written by Miles Bader <miles@gnu.org>
--


local std_table = table -- standard Lua table module


-- module
--
local table = {}


-- Inherit from the standard Lua table module, so clients can just
-- call this module 'table'
--
setmetatable (table, {__index = std_table})


function table.shallow_copy (table)
   local copy = {}
   for k, v in pairs (table) do
      copy[k] = v
   end
   return copy
end


-- return the module
--
return table
