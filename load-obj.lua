-- load-obj.lua -- Load a .obj format mesh
--
--  Copyright (C) 2007, 2008  Miles Bader <miles@gnu.org>
--
-- This source code is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License as
-- published by the Free Software Foundation; either version 3, or (at
-- your option) any later version.  See the file COPYING for more details.
--
-- Written by Miles Bader <miles@gnu.org>
--

local lp = require 'lpeg'
local lu = require 'lpeg-utils'

-- obj-file comment or ignored command
local COMMENT = lp.S"#g" * lu.LINE

function load_obj (filename, mesh, mat_map)
   local mat = mat_map:get_default ()

   -- .obj files use a right-handed coordinate system by convention.
   --
   mesh.left_handed = false

   local function add_vert (x, y, z)
      mesh:add_vertex (x, y, z)
   end

   local function add_poly (v1, v2, v3, ...)
      v1 = v1 - 1	    -- convert vertex indices from 1- to 0-based
      v2 = v2 - 1
      v3 = v3 - 1

      mesh:add_triangle (v1, v2, v3, mat)

      -- Add extra triangles for additional vertices
      --
      local prev = v3
      for i = 1, select ('#', ...) do
	 local vn = select (i, ...) - 1
	 mesh:add_triangle (v1, prev, vn, mat)
	 prev = vn
      end
   end

   local V_CMD
      = lp.P"v" * ((lu.WS_FLOAT * lu.WS_FLOAT * lu.WS_FLOAT) / add_vert)
   local F_CMD
      = lp.P"f" * (lu.WS_INT^2 / add_poly)
   local CMD
      = V_CMD + F_CMD + COMMENT + lu.OPT_HORIZ_WS

   lu.parse_file (filename, CMD * lu.NL)

   return true
end
