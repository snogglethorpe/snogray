-- load-ug.lua -- Load a .ug ("Berkeley UniGrafix") format mesh
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

-- ug-file comment
local p_ws_comment = P"{" * (1 - P"}")^0 * P"}"

local p_hws = lu.p_opt_horiz_ws * ((p_ws_comment * lu.p_opt_horiz_ws)^0)

-- whitespace followed by float
local p_ws_float = p_hws * lu.p_float

-- whitespace followed by name
local p_ws_name = p_hws * C(R("AZ","az", "__") * R("AZ","az", "09", "__")^0)

function load_ug (filename, mesh, mat_dict)
   local def_mat = mat_dict:get_default ()
   local named_mats = {}
   local named_verts = {}

   local function add_mat (name, r, g, b)
      if not mat_dict:contains (name) then
	 named_mats[name] = lambert (color(r,g,b))
	 print ("named mat", name, color(r,g,b), named_mats[name])
      end
   end

   local function add_vert (name, x, y, z)
      named_verts[name] = mesh:add_vertex (x, y, z)
   end

   local function map_vert (name)
      local vert = named_verts[name]
      if not vert then
	 error (filename ": Unknown vertex name \"" .. name .. "\"", 0)
      end
      return vert
   end

   local function map_mat (name)
      if name then
	 local mat = mat_dict:get (name)
	 if not mat then
	    mat = named_mats[name]
	 end
	 if not mat then
	    error (filename ": Unknown material name \"" .. name .. "\"", 0)
	 end
	 return mat
      else
	 return def_mat
      end
   end

   local function add_face (v1, v2, v3, ...)
      local mat = map_mat (select (select ('#', ...), ...))

      v1 = map_vert (v1)
      v2 = map_vert (v2)
      v3 = map_vert (v3)

      mesh:add_triangle (v1, v2, v3, mat)

      -- Add extra triangles for additional vertices
      --
      local prev = v3
      for i = 1, select ('#', ...) - 1 do
	 local vn = map_vert (select (i, ...))
	 mesh:add_triangle (v1, prev, vn, mat)
	 prev = vn
      end
   end

   local p_c_cmd
      = P"c" * ((p_ws_name * p_ws_float * p_ws_float * p_ws_float) / add_mat)
   local p_v_cmd
      = P"v" * ((p_ws_name * p_ws_float * p_ws_float * p_ws_float) / add_vert)
   local p_f_cmd
      = P"f" * p_hws * ((P"(" * p_ws_name^3 * p_hws * P")" * p_ws_name^0)
			/ add_face)
   local p_w_cmd  -- ignored
      = P"w" * (1 - P";")^0
   local p_cmd = (p_v_cmd + p_f_cmd + p_c_cmd) * P";" * p_hws
   local p_line = p_cmd + p_hws

   lu.parse_file (filename, p_line * lu.p_nl)

   return true
end
