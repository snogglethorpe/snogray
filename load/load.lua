-- load.lua -- Scene/mesh loading
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

-- module
--
local load = {}


-- imports
--
local filename = require 'snogray.filename'
local swig = require 'snogray.swig'
local raw = require 'snogray.snograw'


----------------------------------------------------------------
-- Current load-file/directory management
--
-- We keep track of the name of the current file being loaded and its
-- directory.  Filename loading functions can then use this
-- information to look in the same location for other files.
--

local load_filename_stack = {}

-- Push NAME onto the stack of filenames being loaded.
--
function load.push_load_filename (name)
   load_filename_stack[#load_filename_stack + 1] = name
end

-- Pop the top of the stack of filenames being loaded.
--
function load.pop_load_filename (name)
   load_filename_stack[#load_filename_stack] = nil
end

-- Return the filename of the file currently being loaded, or nil if none.
--
function load.cur_load_filename ()
   return load_filename_stack[#load_filename_stack]
end

-- Return the directory of the file currently being loaded, or nil if none.
--
function load.cur_load_directory ()
   local cur_file = load_filename_stack[#load_filename_stack]
   return cur_file and filename.directory (cur_file)
end

-- If OBJ is a string, and a relative filename, then prepend the
-- current load-directory to it, and return the result.  Non-strings
-- and absolute filenames are returned as-is.
--
function load.filename_in_cur_load_directory (obj)
   if obj and type (obj) == 'string' then
      local cur_dir = load.cur_load_directory ()
      local old = obj
      obj = filename.in_directory (obj, cur_dir)
   end

   return obj
end


-- Return LOADER inside a wrapper with the same interface which 
-- (1) prepends the current load-directory to the filename argument to
-- LOADER, and (2) pushes that filename on the load-directory stack
-- while calling LOADER, popping it after LOADER returns.  The return
-- value from LOADER is returned.
--
local function wrap_loader_with_cur_load_file_manager (loader)
   return function  (name, ...)
	       name = load.filename_in_cur_load_directory (name)
	       load.push_load_filename (name)
	       local rval = loader (name, ...)
	       load.pop_load_filename ()
	       return rval
	    end
end


----------------------------------------------------------------
-- Autoloading
--

-- Add an "autoloader" for file format FORMAT to LOADER_TABLE, which
-- when invoked will require the Lua module LOADER_MODULE_NAME, and
-- call a function called "load" in the module to do the loading.  At
-- that time, it will also install the function into LOADER_TABLE so
-- that it will be called directly for subsequent files of the same
-- type.
--
local function add_loader_autoload (loader_table, format, loader_module_name)
   loader_table[format]
      = function (...)
	   -- Load the module.
	   --
	   local module = require (loader_module_name)

	   -- Get the function "load", which the module should define.
	   --
	   local loader = module.load
	   if loader then
	      -- Add a wrapper around LOADER that manages the
	      -- current load directory.
	      --
	      loader = wrap_loader_with_cur_load_file_manager (loader)

	      -- Remember LOADER so that next time it will be
	      -- called directly.
	      --
	      loader_table[format] = loader

	      -- Call LOADER to actually load the file.
	      --
	      return loader (...)
	   else
	      error ("module "..loader_module_name
		     .." doesn't contain a function called 'load'")
	   end
	end
end


----------------------------------------------------------------
-- Common loader support functions
--

-- Add a loader to LOADER_TABLE for file format FORMAT, which loads it
-- as if it had format ALIAS.
--
local function add_loader_alias (loader_table, format, alias)
   loader_table[format] = function (...) return loader_table[alias] (...) end
end

-- Add a loader to LOADER_TABLE for file format FORMAT, which just calls
-- FUN to do the loading.
--
local function add_loader (loader_table, format, fun)
   -- directly set the loader function
   loader_table[format] = wrap_loader_with_cur_load_file_manager (fun)
end


----------------------------------------------------------------
-- Scene loading
--


-- Table of scene loaders for various file extensions.
--
local scene_loaders = {}


-- Load a scene from SCENE_FILE into the scene environment ENVIRON.
--
function load.scene (scene_file, environ)
   local params = environ.params or {}

   local fmt = params.format or filename.extension (scene_file)

   if fmt then
      fmt = string.lower (fmt)
      local loader = scene_loaders[fmt]

      if loader then

	 -- For old versions of SWIG, we need to gc-protect objects
	 -- handed to the scene.
	 --
	 if swig.need_obj_gc_protect then
	    local scene_contents = environ.scene

	    if not swig.has_index_wrappers (scene_contents) then
	       local wrap = swig.index_wrappers (scene_contents)

	       function wrap:add (thing)
		  swig.gc_ref (self, thing)
		  return swig.nowrap_meth_call (self, "add", thing)
	       end
	    end
	 end

	 -- Call the loader.
	 --
	 local ok, err_msg = pcall (loader, scene_file, environ)
	 if not ok then
	    -- Prefix ERR_MSG with "SCENE_FILE: ", unless it already
	    -- seems to contain such a prefix (parsing code, for
	    -- instance often adds the scene_file and line-number etc).
	    if string.sub (err_msg, 1, #scene_file + 1) ~= scene_file..":" then
	       err_msg = scene_file..": "..err_msg
	    end
	    -- propagate the error
	    error (err_msg, 0)
	 end

	 collectgarbage ()
      else
	 error ("unknown scene format \""..fmt.."\"", 0)
      end
   else
      error ("cannot determine scene format for \""..scene_file.."\"", 0)
   end
end


-- Add an "autoloader" for scene file format FORMAT, which when
-- invoked will require the Lua module LOADER_MODULE_NAME, and call a
-- function called "load" in the module to do the loading.  At that
-- time, it will also install the function into the scene-loader table
-- so that it will be called directly for subsequent scene files of
-- the same type.
--
function load.add_scene_loader_autoload (format, loader_module_name)
   add_loader_autoload (scene_loaders, format, loader_module_name)
end

-- Add a loader to LOADER_TABLE for file format FORMAT, which loads it
-- as if it had format ALIAS.
--
function load.add_scene_loader_alias (format, alias)
   add_loader_alias (scene_loaders, format, alias)
end

-- Add a loader to LOADER_TABLE for file format FORMAT, which just
-- calls FUN to do the loading.
--
function load.add_scene_loader (format, fun)
   add_loader (scene_loaders, format, fun)
end


----------------------------------------------------------------
-- Mesh loading
--

-- Table of mesh loaders for various file extensions.
--
local mesh_loaders = {}


-- Load a mesh from SCENE_FILE into MESH.
--
-- Return true for a successful load, false if SCENE_FILE is not
-- recognized as loadable, or an error string if an error occured
-- during loading.
--
-- Note that this only handles formats loaded using Lua, not those
-- handled by the C++ core.  To load any supported format, use the
-- mesh "load" method.
--
function load.mesh (mesh_file, mesh, params)
   params = params or {}

   local fmt = params.format or filename.extension (mesh_file)
   if fmt then
      fmt = string.lower (fmt)
      local loader = mesh_loaders[fmt]
      if loader then
	 loader (mesh_file, mesh, params)
      else
	 error ("unknown mesh format \""..fmt.."\"", 0)
      end
   else
      error ("cannot determine mesh format for \""..mesh_file.."\"", 0)
   end
end


-- Add an "autoloader" for mesh file format FORMAT, which when invoked
-- will require the Lua module LOADER_MODULE_NAME, and call a function
-- called "load" in the module to do the loading.  At that time, it
-- will also install the function into the mesh-loader table so that
-- it will be called directly for subsequent mesh files of the same
-- type.
--
function load.add_mesh_loader_autoload (format, loader_module_name)
   add_loader_autoload (mesh_loaders, format, loader_module_name)
end

-- Add a loader to LOADER_TABLE for file format FORMAT, which loads it
-- as if it had format ALIAS.
--
function load.add_mesh_loader_alias (format, alias)
   add_loader_alias (mesh_loaders, format, alias)
end

-- Add a loader to LOADER_TABLE for file format FORMAT, which just
-- calls FUN to do the loading.
--
function load.add_mesh_loader (format, fun)
   add_loader (mesh_loaders, format, fun)
end


----------------------------------------------------------------
-- Various loaders
--

local add_scene_loader = load.add_scene_loader
local add_scene_loader_autoload = load.add_scene_loader_autoload
local add_scene_loader_alias = load.add_scene_loader_alias

local add_mesh_loader = load.add_mesh_loader
local add_mesh_loader_autoload = load.add_mesh_loader_autoload
local add_mesh_loader_alias = load.add_mesh_loader_alias

-- Scene formats with Lua loaders.
--
add_scene_loader_autoload ("lua", "snogray.loader.scene.lua")
add_scene_loader_alias ("luac", "lua")
add_scene_loader_alias ("luo", "lua")
add_scene_loader_autoload ("nff", "snogray.loader.scene.nff")
add_scene_loader_autoload ("pbrt", "snogray.loader.scene.pbrt")

-- Scene formats with C loaders
--
add_scene_loader ("3ds", function (scene_file, environ)
			    raw.load_3ds_file (scene_file,
					       environ.scene, environ.camera)
			 end)

-- Mesh formats with Lua loaders.
--
add_mesh_loader_autoload ("obj", "snogray.loader.mesh.obj")
add_mesh_loader_autoload ("ug", "snogray.loader.mesh.ug")
add_mesh_loader_autoload ("stl", "snogray.loader.mesh.stl")

-- Mesh formats with C loaders.
--
add_mesh_loader ("ply", raw.load_ply_file)
add_mesh_loader ("msh", raw.load_msh_file)
add_mesh_loader ("3ds", raw.load_3ds_file)


-- return the module
--
return load
