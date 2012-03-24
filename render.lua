-- render.lua -- Rendering-related functions
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
local render = {}

local raw = require "snogray.snograw"


render.global_state = raw.GlobalRenderState
render.context = raw.RenderContext
render.manager = raw.RenderMgr
render.pattern = raw.RenderPattern
render.stats = raw.RenderStats


-- return module
--
return render
