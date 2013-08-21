-- load-lua.lua -- Loading of scenes written in Lua
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

local lua = {}  -- module


-- Load a Lua scene description from SCENE_FILE into the scene
-- environment ENVIRON.
--
function lua.load (scene_file, environ)
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

   -- Finally, evaluate the loaded file!
   --
   contents ()
end


return lua
