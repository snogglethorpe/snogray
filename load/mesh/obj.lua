-- load-obj.lua -- Load a .obj format mesh
--
--  Copyright (C) 2007, 2008, 2011-2013  Miles Bader <miles@gnu.org>
--
-- This source code is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License as
-- published by the Free Software Foundation; either version 3, or (at
-- your option) any later version.  See the file COPYING for more details.
--
-- Written by Miles Bader <miles@gnu.org>
--

local obj = {}  -- module


local lp = require 'lpeg'
local lu = require 'snogray.lpeg-utils'

-- obj-file comment or ignored command
local COMMENT = lp.S"#go" * lu.LINE
local WS = lu.REQ_HORIZ_WS
local OPT_WS = lu.OPT_HORIZ_WS


function obj.load (filename, mesh, mat_map)
   local mat = mat_map:get_default ()

   -- .obj files use a right-handed coordinate system by convention.
   --
   mesh.left_handed = false

   local function add_vert (x, y, z)
      mesh:add_vertex (x, y, z)
   end

   local norm_index = 0
   local function add_norm (x, y, z)
      mesh:add_normal (norm_index, x, y, z)
      norm_index = norm_index + 1
   end

   -- We only support files where the normal indices are identical to
   -- the vertex indices, so when both are specified, check our
   -- assumption.
   --
   local function check_indices (vi, ni)
      if ni and ni ~= vi then
	 lu.parse_err ("Normal indices must be identical to vertex indices")
      end
      return vi
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

   local function load_mtllib (name)
      lu.parse_warn ("ignoring mtllib \"" .. name .. "\"")
   end

   local WS_VERT_INDEX
      = (lu.WS_INT * (OPT_WS * lp.P"//" * lu.WS_INT)^-1) / check_indices
   local V_CMD
      = lp.P"v" * ((lu.WS_FLOAT * lu.WS_FLOAT * lu.WS_FLOAT) / add_vert)
   local VN_CMD
      = lp.P"vn" * ((lu.WS_FLOAT * lu.WS_FLOAT * lu.WS_FLOAT) / add_norm)
   local F_CMD
      = lp.P"f" * (WS_VERT_INDEX^2 / add_poly)
   local MTLLIB_CMD
      = lp.P"mtllib" * WS * (lu.LINE / load_mtllib)
   local USEMTL_CMD
      = lp.P"usemtl" * WS * lu.LINE
   local CMD
      = V_CMD + VN_CMD + F_CMD + MTLLIB_CMD + USEMTL_CMD + COMMENT + OPT_WS

   lu.parse_file (filename, CMD * lu.NL)
end


return obj
