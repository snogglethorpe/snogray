-- load-stl.lua -- Load a .stl (stereolithography) format mesh
--
--  Copyright (C) 2007, 2008, 2011, 2012  Miles Bader <miles@gnu.org>
--
-- This source code is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License as
-- published by the Free Software Foundation; either version 3, or (at
-- your option) any later version.  See the file COPYING for more details.
--
-- Written by Miles Bader <miles@gnu.org>
--

local lp = require 'lpeg'
local lu = require 'snogray.lpeg-utils'
local surface = require 'snogray.surface'

-- local abbreviations for lpeg primitives
local P, R, S, C = lpeg.P, lpeg.R, lpeg.S, lpeg.C

local HWS = lu.OPT_HORIZ_WS
local OPT_WS = lu.OPT_WS

-- whitespace followed by float
local WS_FLOAT = HWS * lu.FLOAT

function load_stl (filename, mesh, mat_dict)
   local mat = mat_dict:get_default ()

   local facet_verts = {}
   local vg = surface.mesh_vertex_group ()

   local function add_vert (x, y, z)
      facet_verts[#facet_verts + 1] = mesh:add_vertex (x, y, z, vg)
   end

   local function add_facet ()
      local fv = facet_verts

      if #fv < 3 then
	 lu.parse_err ("not enough vertices in facet")
      end

      mesh:add_triangle (fv[1], fv[2], fv[3], mat)

      -- Add extra triangles for additional vertices
      --
      local prev = fv[3]
      for i = 4, #fv do
	 local vn = fv[i]
	 mesh:add_triangle (fv[1], prev, vn, mat)
	 prev = vn
      end

      facet_verts = {}
   end

   local SYNC = OPT_WS * lu.ERR_POS
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

   lu.parse_file (filename, SOLID)
end
