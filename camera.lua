-- camera.lua -- Lua camera handling for snogray
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
local camera = {}

-- make the module callable, as a shortcut for "camera.new"
--
setmetatable (camera, { __call = function (m, ...) return m.new (...) end })

-- imports
--
local raw = require "snogray.snograw"


camera.new = raw.Camera


-- return module
--
return camera
