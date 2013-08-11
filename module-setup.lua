-- module-setup.lua -- Lua module system tweaking for snogray
--
--  Copyright (C) 2012, 2013  Miles Bader <miles@gnu.org>
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

-- Various directories
--
local lua_dir, lua_module_dir, lua_loader_dir

if installed_lua_root then
   --
   -- We're operating in "installed mode".  
   --

   -- Various directories in their proper locations.
   --
   lua_dir = installed_lua_root
   lua_module_dir = lua_dir.."/module"
   lua_loader_dir = lua_dir.."/loader"

   -- Push the installation Lua directory onto the front of the module
   -- search path.
   --
   package.path = lua_module_dir.."/?.lua;"..package.path
else
   --
   -- We're operating in "uninstalled mode".  Add a package searcher
   -- that just tries to load anything prefixed with "snogray." from
   -- (1) a subdirectory of the current directory with the same name
   -- as the module (so for a module spec "snogray.PKG.PKG", it will
   -- try to load "PKG/PKG.lua"), and if that fails, then (2) just the
   -- current directory ("PKG.lua").
   --

   -- A table mapping module names to their locations in the source
   -- tree, for cases which can't be guessed based on the name.
   --
   local uninst_module_mapping = {
      accel = "space/accel",
      cmdlineparser = "cli/cmdlineparser",
      coord = "geometry/coord",
      ["image-sampled-output-cmdline"]
	 = "imagecli/image-sampled-output-cmdline",
      transform = "geometry/transform"
   }

   local function load_uninstalled_snogray_package (pkg, ...)
      local snogray_pkg = string.match (pkg, "^snogray[.](.*)$")
      if snogray_pkg then
	 return function ()
		   local thunk, err

		   -- See if this name has an explicitly recorded location
		   local mapped = uninst_module_mapping[snogray_pkg]
		   if mapped then
		      -- If so, just use it
		      snogray_pkg = mapped
		   else
		      -- Otherwise first try loading from a
		      -- subdirectory of the same name as the module
		      thunk, err
			 = loadfile (snogray_pkg.."/"..snogray_pkg..".lua")
		   end

		   -- Try loading from the current directory
		   if not thunk then
		      thunk, err = loadfile (snogray_pkg..".lua")
		   end
		   if not thunk then
		      error (err, 0)
		   end
		   return thunk ()
		end
      else
	 return nil
      end
   end

   local searchers = package.loaders or package.searchers
   table.insert (searchers, 1, load_uninstalled_snogray_package)

   -- Everything in current directory.
   --
   lua_dir = "."
   lua_module_dir = "."
   lua_loader_dir = "."
end

-- Setup a "module" which stores various snogray environment parameters.
--
-- Currently this contains the following definitions:
--
--   lua_dir		The root of the tree where snogray Lua files are
--			stored.
--
--   lua_module_dir	The directory where snogray Lua modules are stored.
--
--   lua_loader_dir	The directory where snogray Lua scene/mesh
--			loaders are stored.
--
--   version            Snogray version string (added in C code)
--
local snogray_environ = {}
snogray_environ.lua_dir = lua_dir
snogray_environ.lua_module_dir = lua_module_dir
snogray_environ.lua_loader_dir = lua_loader_dir

package.loaded['snogray.environ'] = snogray_environ
