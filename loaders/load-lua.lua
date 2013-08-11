-- load-lua.lua -- Loading of scenes written in Lua
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

-- imports
--
local filename = require 'snogray.filename'


----------------------------------------------------------------
-- Specialized file-loading functions for scene-files
--

local include_path = { "." }

local function load_include (scene_file, env)
   local loaded, loaded_filename, err_msg

   if not filename.extension (scene_file) then
      scene_file = scene_file .. ".lua"
   end

   --
   -- Note: in the calls to loadfile below, we pass ENV to set the
   -- loaded file's environment in Lua 5.2; the extra arguments to
   -- loadfile are ignored in Lua 5.1 (setting the environment in Lua
   -- 5.1 is handled with setfenv in eval_include).
   --

   if string.sub (scene_file, 1, 1) == "/" then
      loaded_filename = scene_file
      loaded, err_msg = loadfile (scene_file, nil, env)
   else
      -- First try the same directory as cur_filename.
      --
      local cur_dir = filename.directory (env.cur_filename)
      if cur_dir then
	 loaded_filename = cur_dir .. "/" .. scene_file
	 loaded, err_msg = loadfile (loaded_filename, nil, env)
      end

      -- If we didn't find anything, try searching along include_path.
      --
      if not loaded then
	 local path_pos = 1
	 while not loaded and path_pos <= #include_path do
	    loaded_filename = include_path[path_pos] .. "/" .. scene_file
	    loaded, err_msg = loadfile (loaded_filename, nil, env)
	    path_pos = path_pos + 1
	 end
      end
   end

   return loaded, loaded_filename, err_msg
end

local function eval_include (loaded, env, loaded_filename, err_msg)
   if loaded then
      local old_cur_filename = env.cur_filename
      env.cur_filename = loaded_filename

      -- In Lua 5.1, set the environment of the loaded chunk (in Lua
      -- 5.2, this is done during loading instead).
      --
      if setfenv then
	 setfenv (loaded, env)
      end

      -- Evaluate the loaded chunk.
      --
      loaded ()

      env.cur_filename = old_cur_filename
   else
      error (err_msg)
   end
end


-- Load SCENE_FILE evaluated in the environment ENV.  SCENE_FILE is
-- searched for using the path in the "include_path" variable; while
-- it is being evaluating, the directory SCENE_FILE was actually
-- loaded from is prepended to "include_path", so that any recursive
-- includes may come from the same directory.
--
local function include_file (scene_file, env)
   local loaded, loaded_filename, err_msg = load_include (scene_file, env)
   eval_include (loaded, env, loaded_filename, err_msg)
   return loaded_filename
end


-- Load SCENE_FILE into the environment ENV.  This is like
-- "include_file", but multiple attempts to load the same file are
-- suppressed (the variable "_used_files" in ENV is used to record
-- which use file have already been loaded).
--
local function use_file (scene_file, env)
   local loaded, loaded_filename, err_msg = load_include (scene_file, env)

   if not env._used_files then
      env._used_files = {}
   end

   if not env._used_files[loaded_filename] then
      env._used_files[loaded_filename] = true

      eval_include (loaded, env, loaded_filename, err_msg)
      return loaded_filename
   end
end


----------------------------------------------------------------
-- Lua file loader
--


-- A metatable for inheriting from the "all-in-one" environment
-- 
local inherit_all_in_one_metatable = { __index = require 'snogray.all-in-one' }


-- Load Lua scene description from SCENE_FILE into SCENE and CAMERA.
--
function load_lua (scene_file, scene, camera, params)
   -- Make a new environment to evaluate the file contents in; it will
   -- inherit from the "all-in-one" scene interface for convenience.
   -- There are no global pointers to this table so it and its
   -- contents will be garbage collected after loading.
   --
   local environ = {}
   setmetatable (environ, inherit_all_in_one_metatable)

   -- Load the user's file!  This just constructs a function from the
   -- loaded file, but doesn't actually evaluate it.  We pass ENVIRON
   -- to set the loaded file's environment in Lua 5.2; the extra
   -- arguments to loadfile are ignored in Lua 5.1 (setting the
   -- environment in Lua 5.1 is handled with setfenv below)
   --
   local contents, err = loadfile (scene_file, nil, environ)

   -- If there was a loading error, propagate it.
   --
   if not contents then
      error (err, 0)
   end

   -- In Lua 5.1, set the environment of the loaded chunk (in Lua 5.2,
   -- this is done during loading instead).
   --
   if setfenv then
      setfenv (contents, environ)
   end

   -- Add references to the scene, camera, and parameters.
   --
   environ.scene = scene
   environ.camera = camera
   environ.params = params

   -- Remember scene_file being loaded, so we can find other files in
   -- the same location.
   --
   environ.cur_filename = scene_file

   -- Add specialized file-loading functions that know what
   -- environment to use.
   --
   environ.include = function (file) include_file (file, environ) end
   environ.use = function (file) use_file (file, environ) end

   -- Finally, evaluate the loaded file!
   --
   contents ()
end
