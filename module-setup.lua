-- module-setup.lua -- Lua module system tweaking for snogray
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

-- Our one argument is the directory where we can find our installed
-- Lua files, or nil if the current directory should be used to find
-- uninstalled Lua files instead.
--
local installed_lua_root = ...

if installed_lua_root then
   --
   -- We're operating in "installed mode".  Push the installation Lua
   -- directory onto the front of the module search path.
   --
   package.path = installed_lua_root.."/?.lua;"..package.path
else
   --
   -- We're operating in "uninstalled mode".  Add a package searcher
   -- that just tries to load anything prefixed with "snogray." from
   -- the current directory.
   --
   local function load_uninstalled_snogray_package (pkg, ...)
      local snogray_pkg = string.match (pkg, "^snogray[.](.*)$")
      if snogray_pkg then
	 return function () return loadfile (snogray_pkg..".lua") () end
      else
	 return nil
      end
   end

   table.insert (package.loaders, 1, load_uninstalled_snogray_package)
end
