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

-- Our two arguments are (1) a boolean which is true if we're
-- operating in "installed mode" (running an installed snogray rather
-- than a snogray in the build directory), and (2) the appropriate
-- "snogray root" directory for the mode we're operating in.  The
-- latter is the snogray Lua install directory in installed mode, and
-- the root of the source directory in uninstalled mode; in either
-- case, we look for our Lua files relative to that location.
--
local installed_mode, lua_root = ...

if installed_mode then
   --
   -- We're operating in "installed mode".  Push the installation Lua
   -- directory onto the front of the module search path, so the
   -- normal Lua module search mechanism can find them.
   --
   package.path = lua_root.."/module/?.lua;"..package.path
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
      ["accel"]		 = "space/accel",
      ["camera"]	 = "scene/camera",
      ["camera-cmdline"] = "scene/camera-cmdline",
      ["cmdlineparser"]	 = "cli/cmdlineparser",
      ["coord"]		 = "geometry/coord",
      ["file"]		 = "util/file",
      ["filename"]	 = "util/filename",
      ["image-sampled-output-cmdline"] = "image/image-sampled-output-cmdline",
      ["lpeg-utils"]	 = "util/lpeg-utils",
      ["render"]	 = "render-mgr/render",
      ["render-cmdline"] = "render-mgr/render-cmdline",
      ["scene-cmdline"]  = "scene/scene-cmdline",
      ["string"]	 = "util/string",
      ["string-fill"]	 = "util/string-fill",
      ["swig"]		 = "util/swig",
      ["sys"]		 = "util/sys",
      ["table"]		 = "util/table",
      ["transform"]	 = "geometry/transform",
   }

   local function load_uninstalled_snogray_package (pkg, ...)
      local snogray_pkg = string.match (pkg, "^snogray[.](.*)$")
      if snogray_pkg then
	 local filebase = string.gsub (snogray_pkg, "[.]", "/")
	 return function ()
		   local thunk, err

		   -- See if this name has an explicitly recorded location
		   local mapped = uninst_module_mapping[filebase]
		   if mapped then
		      -- If so, just use it
		      filebase = mapped
		   else
		      -- See if this is a loader module, which are
		      -- simply loaded from the "load" source
		      -- subdirectory.
		      local loader = string.match (filebase, "^loader/(.*)$")
		      if loader then
			 filebase = "load/"..loader
		      else
			 -- Otherwise first try loading from a
			 -- subdirectory of the same name as the module
			 thunk, err
			    = loadfile (lua_root.."/"..filebase
					.."/"..filebase..".lua")
		      end
		   end

		   -- Try loading from the source directory
		   if not thunk then
		      thunk, err = loadfile (lua_root.."/"..filebase..".lua")
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
end

-- Setup a "module" which stores various snogray environment parameters.
--
-- Currently this contains the following definitions:
--
--   lua_dir		The root of the tree where snogray Lua files are
--			stored.
--
--   version            Snogray version string (added in C code)
--
local snogray_environ = {}
snogray_environ.lua_dir = lua_root

package.loaded['snogray.environ'] = snogray_environ
