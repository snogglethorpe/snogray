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
   -- the snogray source tree, using the following rules:
   --
   --  1. Packages with explicitly recorded filename mappings are
   --     loaded from the corresponding filename (relative to the
   --     source-tree root).
   --  2. Packages of the form "snogray.loader.PKG", are loaded from
   --     "load/PKG.lua", relative to the source-tree root, with all
   --     periods in PKG replaced by slashes (as loaders are often in
   --     subdirectories).
   --  3. Otherwise, for packages of the form "snogray.PKG", the
   --     filenames "PKG/PKG.lua" and PKG.lua" are tried in turn,
   --     again relative to the source-tree root.
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
      ["file"]		 = "lua-util/file",
      ["filename"]	 = "lua-util/filename",
      ["image-sampled-output-cmdline"] = "image/image-sampled-output-cmdline",
      ["lpeg-utils"]	 = "lua-util/lpeg-utils",
      ["render-cmdline"] = "render/render-cmdline",
      ["scene-cmdline"]  = "scene/scene-cmdline",
      ["string"]	 = "lua-util/string",
      ["string-fill"]	 = "lua-util/string-fill",
      ["swig"]		 = "lua-util/swig",
      ["sys"]		 = "lua-util/sys",
      ["table"]		 = "lua-util/table",
      ["transform"]	 = "geometry/transform",
   }

   -- Given the base filename BASE in the Lua source-tree, return the
   -- full filename to try loading a package from.
   --
   local function source_file_name (base)
      return lua_root.."/"..base..".lua"
   end

   -- Given the base filename BASE in the Lua source-tree, compute its
   -- full filename, and try to load it.  If loading is successful,
   -- return two values, the file loader, and the full name of the
   -- file loaded (these are the two values that a Lua package
   -- searcher should return).
   --
   local function tryload (base)
      local filename = source_file_name (base)
      return loadfile (filename), filename
   end

   local function load_uninstalled_snogray_package (pkg, ...)
      --
      -- This searcher only handles snogray packages.
      --
      local snogray_pkg = string.match (pkg, "^snogray[.](.*)$")
      if snogray_pkg then
	 --
	 -- See if this name has an explicitly recorded location
	 --
	 local mapped = uninst_module_mapping[snogray_pkg]
	 if mapped then
	    --
	    -- If so, just load it directly.
	    --
	    return tryload (mapped)
	 else
	    --
	    -- See if this is a loader module, "snogray.loader.*";
	    -- these are simply loaded from the "load" source
	    -- subdirectory.
	    --
	    local loader_pkg = string.match (snogray_pkg, "^loader[.](.*)$")
	    if loader_pkg then
	       --
	       -- Loader modules can be hierarchial, in which case
	       -- they should be loaded from a corresponding
	       -- subdirectory, so just replact dots with slashes.
	       --
	       return tryload ("load/"..string.gsub (loader_pkg, "[.]", "/"))
	    else
	       --
	       -- Otherwise first try loading from a subdirectory of
	       -- the same name as the module.
	       --
	       local fname = source_file_name (snogray_pkg.."/"..snogray_pkg)
	       local fcontents = loadfile (fname)
	       if fcontents then
		  return fcontents, fname
	       else
		  --
		  -- ... and finally, if nothing else worked, try in
		  -- the root of the source tree.
		  -- 
		  return tryload (snogray_pkg)
	       end
	    end
	 end
      end

      return nil
   end

   -- Put our seacher function at the front of the global list of
   -- searchers.
   --
   -- [Note that this puts even before the standard Lua searcher that
   -- handles prelods.  That's not an issue in practice, as we don't
   -- have any preloaded modules that conflict with loaded Lua
   -- modules, but it's something to be aware of.]
   --
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
