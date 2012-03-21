-- load.lua -- Scene/mesh loading
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

-- module
--
local load = {}


-- imports
--
local filename = require 'snogray.filename'
local swig = require 'snogray.swig'
local raw = require 'snogray.snograw'


----------------------------------------------------------------
-- Autoloading
--

-- A metatable for inheriting from the snogray environment
--
local inherit_snogray_metatable = { __index = require 'snogray.snogray' }

-- Add a stub for file-extension EXT to LOADER_TABLE that will load
-- LOADER_FILE and call the function named LOADER_NAME (which
-- LOADER_FILE must define); the stub will also install that function
-- into LOADER_TABLE so that it can be called directly for subsequent
-- files of the same type.
--
local function add_autoload_stub (loader_table, ext, loader_file, loader_name)
   local loader_dir = require ('snogray.environ').lua_loader_dir
   loader_table[ext]
      = function (...)
	   print ("* autoloading: "..loader_file)

	   local contents, err = loadfile (loader_dir.."/"..loader_file)

	   if contents then
	      local environ = {}
	      setmetatable (environ, inherit_snogray_metatable)
	      setfenv (contents, environ)

	      contents ()	-- Finish loading

	      local loader = environ[loader_name]
	      if loader then
		 loader_table[ext] = loader
		 return loader (...)
	      else
		 error ("loading "..loader_file.." didn't define "..loader_name)
	      end
	   else
	      error (err, 0)
	   end
	end
end


----------------------------------------------------------------
-- Common loader support functions
--

-- Add a loader to LOADER_TABLE for FORMAT.
--
-- There are three types of loaders, depending on the value of the
-- remaining arguments:
--
--  (1) If LOADER is a function, it is simply called directly.
--
--  (2) If LOADER is a string, and LOADER_FILE is nil, then LOADER
--      should be another format name; the loader for that format is
--      recursively invoked to do the loading.
--
--  (3) If LOADER is a string, and LOADER_FILE is also a string, then
--      the Lua file LOADER_FILE is loaded into a new environment
--      inheriting from the "snogray.snogray" module, and expected to
--      define a function in that environment called LOADER, which is
--      called to do the loading.
--
local function add_loader (loader_table, format, loader, loader_file)
   if type (loader) == 'string' then
      if loader_file then
	 -- autoload LOADER_FILE to define LOADER
	 add_autoload_stub (loader_table, format, loader_file, loader)
      else
	 -- install a thunk which recursively tries format LOADER
	 loader_table[format]
	    = function (...) return loader_table[loader] (...) end
      end
   else
      loader_table[format] = loader
   end
end


----------------------------------------------------------------
-- Scene loading
--


-- Table of scene loaders for various file extensions.
--
local scene_loaders = {}


-- Load a scene from SCENE_FILE into SCENE and CAMERA.
--
function load.scene (scene_file, scene, camera, params)
   params = params or {}

   local fmt = params.format or filename.extension (scene_file)

   if fmt then
      fmt = string.lower (fmt)
      local loader = scene_loaders[fmt]

      if loader then

	 -- For old versions of SWIG, we need to gc-protect objects
	 -- handed to the scene.
	 --
	 if swig.need_obj_gc_protect and not swig.has_index_wrappers (scene) then
	    local wrap = swig.index_wrappers (scene)

	    function wrap:add (thing)
	       swig.gc_ref (self, thing)
	       return swig.nowrap_meth_call (self, "add", thing)
	    end
	 end

	 -- Call the loader.
	 --
	 local ok, err_msg = pcall (loader, scene_file, scene, camera, params)
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
      else
	 error ("unknown scene format \""..fmt.."\"", 0)
      end
   else
      error ("cannot determine scene format for \""..scene_file.."\"", 0)
   end
end


-- Add a scene loader for FORMAT.
--
-- There are three types of loaders, depending on the value of the
-- remaining arguments:
--
--  (1) If LOADER is a function, it is simply called directly.
--
--  (2) If LOADER is a string, and LOADER_FILE is nil, then LOADER
--      should be another format name; the loader for that format is
--      recursively invoked to do the loading.
--
--  (3) If LOADER is a string, and LOADER_FILE is also a string, then
--      the Lua file LOADER_FILE is loaded into a new environment
--      inheriting from the "snogray.snogray" module, and expected to
--      define a function in that environment called LOADER, which is
--      called to do the loading.
--
function load.add_scene_loader (format, loader, loader_file)
   add_loader (scene_loaders, format, loader, loader_file)
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
-- recognized as loadable, or an error string if an error occured during
-- loading.
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


-- Add a mesh loader for FORMAT.
--
-- There are three types of loaders, depending on the value of the
-- remaining arguments:
--
--  (1) If LOADER is a function, it is simply called directly.
--
--  (2) If LOADER is a string, and LOADER_FILE is nil, then LOADER
--      should be another format name; the loader for that format is
--      recursively invoked to do the loading.
--
--  (3) If LOADER is a string, and LOADER_FILE is also a string, then
--      the Lua file LOADER_FILE is loaded into a new environment
--      inheriting from the "snogray.snogray" module, and expected to
--      define a function in that environment called LOADER, which is
--      called to do the loading.
--
function load.add_mesh_loader (format, loader, loader_file)
   add_loader (mesh_loaders, format, loader, loader_file)
end


----------------------------------------------------------------
-- Various loaders
--

local add_scene_loader = load.add_scene_loader
local add_mesh_loader = load.add_mesh_loader

-- Scene formats with Lua loaders.
--
add_scene_loader ("lua", "load_lua", "load-lua.lua")
add_scene_loader ("luac", "lua")
add_scene_loader ("luo", "lua")
add_scene_loader ("nff", "load_nff", "load-nff.lua")
add_scene_loader ("pbrt", "load_pbrt", "load-pbrt.lua")

-- Scene formats with C loaders
--
add_scene_loader ("3ds", raw.load_3ds_file)

-- Mesh formats with Lua loaders.
--
add_mesh_loader ("obj", "load_obj", "load-obj.lua")
add_mesh_loader ("ug", "load_ug", "load-ug.lua")
add_mesh_loader ("stl", "load_stl", "load-stl.lua")

-- Mesh formats with C loaders.
--
add_mesh_loader ("ply", raw.load_ply_file)
add_mesh_loader ("msh", raw.load_msh_file)
add_mesh_loader ("3ds", raw.load_3ds_file)


-- return the module
--
return load
