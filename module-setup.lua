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

-- remove the current directory from the module search path; it will be
-- re-added later if necessary.
--
package.path = string.gsub (package.path, ';[.]/[?][.]lua(;?)', '%1')
package.path = string.gsub (package.path, '^[.]/[?][.]lua;', '')

if installed_lua_root then
   --
   -- We're operating in "installed mode".  Push the installation Lua
   -- directory onto the front of the module search path.
   --
   package.path = installed_lua_root.."/?.lua;"..package.path
else
   --
   -- We're operating in "uninstalled mode".  Push the current
   -- directory onto the front of the module search path, and add a
   -- package searcher that strips "snogray." off the front of module
   -- requests.
   --
   package.path = "./?.lua;"..package.path

   -- Add a package searcher that strips "snogray." off the front of
   -- module requests.
   --
   local function load_uninstalled_snogray_package (package)
      local snogray_package = string.match (package, '^snogray[.](.*)$')
      if snogray_package then
	 return function () return require (snogray_package) end
      else
	 return nil
      end
   end

   table.insert (package.loaders, 1, load_uninstalled_snogray_package)
end
