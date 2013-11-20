-- load-stl.lua -- Load a .stl (stereolithography) format mesh
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

local stl = {} -- module


local lpeg = require 'lpeg'
local lpeg_utils = require 'snogray.lpeg-utils'
local surface = require 'snogray.surface'
local vector = require 'snogray.vector'


-- local abbreviations for lpeg primitives
local P, R, S, C = lpeg.P, lpeg.R, lpeg.S, lpeg.C

local HWS = lpeg_utils.OPT_HORIZ_WS
local OPT_WS = lpeg_utils.OPT_WS

-- whitespace followed by float
local WS_FLOAT = HWS * lpeg_utils.FLOAT


function stl.load (filename, mesh, part)
   local facet_verts = {}
   local vg = surface.mesh_vertex_group ()

   local triangle_vertex_indices = vector.unsigned ()

   local function add_vert (x, y, z)
      facet_verts[#facet_verts + 1] = mesh:add_vertex (x, y, z, vg)
   end

   local function add_facet ()
      local fv = facet_verts

      if #fv < 3 then
	 lpeg_utils.parse_err ("not enough vertices in facet")
      end

      triangle_vertex_indices:add (fv[1], fv[2], fv[3])

      -- Add extra triangles for additional vertices
      --
      local prev = fv[3]
      for i = 4, #fv do
	 local vn = fv[i]
	 triangle_vertex_indices:add (fv[1], prev, vn)
	 prev = vn
      end

      -- clear FACET_VERTS
      --
      for i = 1, #fv do
	 fv[i] = nil
      end
   end

   local SYNC = OPT_WS * lpeg_utils.ERR_POS
   local COORDS = WS_FLOAT * WS_FLOAT * WS_FLOAT
   local VERTEX = P"vertex" * (COORDS / add_vert)
   local NORMAL = P"normal" * COORDS
   local LOOP_EL = VERTEX
   local LOOP
      = (P"outer" * HWS * P"loop"
	 * (SYNC * LOOP_EL)^0 * OPT_WS
         * P"end" * HWS * P"loop")
   local FACET_EL = LOOP + NORMAL
   local FACET
      = (P"facet"
	 * (SYNC * FACET_EL)^0 * OPT_WS
	 * P"end" * HWS * P"facet") / add_facet
   local SOLID_EL = FACET
   local SOLID
      = (P"solid" * HWS * P"ascii"
	 * (SYNC * SOLID_EL)^0 * OPT_WS
         * P"end" * HWS * P"solid" * OPT_WS)

   lpeg_utils.parse_file (filename, SOLID)

   mesh:add_triangles (part, triangle_vertex_indices, 0)
end


return stl
