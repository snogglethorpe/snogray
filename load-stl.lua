-- load-stl.lua -- Load a .stl (stereolithography) format mesh
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

-- local abbreviations for lpeg primitives
local P, R, S, C = lpeg.P, lpeg.R, lpeg.S, lpeg.C

local p_hws = lu.p_opt_horiz_ws
local p_opt_ws = lu.p_opt_ws

-- whitespace followed by float
local p_ws_float = p_hws * lu.p_float

function load_stl (filename, mesh, mat_dict)
   local mat = mat_dict:get_default ()

   local facet_verts = {}
   local vg = mesh_vertex_group ()

   local err_pos = 1
   local err_text = nil
   local function record_err_pos (text, pos)
      err_text = text
      err_pos = pos
      return pos
   end
   local function get_err_pos ()
      return err_pos
   end

   local function add_vert (x, y, z)
      facet_verts[#facet_verts + 1] = mesh:add_vertex (x, y, z, vg)
   end

   local function add_facet ()
      local fv = facet_verts

      if #fv < 3 then
	 lu.parse_err (err_text, err_pos, "not enough vertices in facet")
      end

      mesh:add_triangle (fv[1], fv[2], fv[3], mat)

      -- Add extra triangles for additional vertices
      --
      local prev = fv[3]
      for i = 4, #fv do
	 local vn = fv[i]
	 mesh:add_triangle (v1, prev, vn, mat)
	 prev = vn
      end

      facet_verts = {}
   end

   local p_err_pos = P(record_err_pos)
   local p_sync = p_opt_ws * p_err_pos
   local p_coords = p_ws_float * p_ws_float * p_ws_float
   local p_vertex = P"vertex" * (p_coords / add_vert)
   local p_normal = P"normal" * p_coords
   local p_loop_el = p_vertex
   local p_loop
      = (P"outer" * p_hws * P"loop"
	 * (p_sync * p_loop_el)^0 * p_opt_ws
         * P"end" * p_hws * P"loop")
   local p_facet_el = p_loop + p_normal
   local p_facet
      = (P"facet"
	 * (p_sync * p_facet_el)^0 * p_opt_ws
	 * P"end" * p_hws * P"facet") / add_facet
   local p_solid_el = p_facet
   local p_solid
      = (P"solid" * p_hws * P"ascii"
	 * (p_sync * p_solid_el)^0 * p_opt_ws
         * P"end" * p_hws * P"solid" * p_opt_ws)

   lu.parse_file (filename, p_solid, get_err_pos)

   return true
end
