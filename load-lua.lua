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
local snogray = require 'snogray.snogray'


-- A metatable for inheriting from the snogray environment
-- 
local inherit_snogray_metatable = { __index = require 'snogray.snogray' }


-- Load Lua scene description from SCENE_FILE into SCENE and CAMERA.
--
function load_lua (scene_file, scene, camera, params)

   -- Load the user's file!  This just constructs a function from the
   -- loaded file, but doesn't actually evaluate it.
   --
   local contents, err = loadfile (scene_file)

   if not contents then
      error (err, 0)		-- propagate the loading error
   end

   -- Make a new environment to evaluate the file contents in; it will
   -- inherit from "snogray" for convenience.  There are no global
   -- pointers to this table so it and its contents will be garbage
   -- collected after loading.
   --
   local environ = {}
   setmetatable (environ, inherit_snogray_metatable)
   setfenv (contents, environ)

   -- Add references to the scene, camera, and parameters.
   --
   environ.scene = scene
   environ.camera = camera
   environ.params = params

   -- Remember scene_file being loaded, so we can find other files in
   -- the same location.
   --
   environ.cur_filename = scene_file

   -- Finally, evaluate the loaded file!
   --
   contents ()
end
