-- coord.lua -- Coordinate support
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
local coord = {}

-- imports
--
local raw = require "snogray.snograw"


coord.pos = raw.Pos
coord.vec = raw.Vec
coord.bbox = raw.BBox
coord.ray = raw.Ray
coord.uv = raw.UV
coord.frame = raw.Frame

coord.origin = coord.pos (0, 0, 0)

coord.midpoint = raw.midpoint
coord.dot = raw.dot
coord.cross = raw.cross


-- return the module
--
return coord
