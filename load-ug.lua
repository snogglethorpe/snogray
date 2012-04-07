-- load-ug.lua -- Load a .ug ("Berkeley UniGrafix") format mesh
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

local lpeg = require 'lpeg'
local lu = require 'snogray.lpeg-utils'
local color = require 'snogray.color'
local material = require 'snogray.material'

-- local abbreviations for lpeg primitives
local P, R, S, C = lpeg.P, lpeg.R, lpeg.S, lpeg.C

-- ug-file comment
local WS_COMMENT = P"{" * (1 - P"}")^0 * P"}"

local HWS = lu.OPT_HORIZ_WS * ((WS_COMMENT * lu.OPT_HORIZ_WS)^0)

-- whitespace followed by float
local WS_FLOAT = HWS * lu.FLOAT

-- whitespace followed by name
local WS_NAME = HWS * C(R("AZ","az", "__") * R("AZ","az", "09", "__")^0)

function load_ug (filename, mesh, mat_dict)
   local def_mat = mat_dict:get_default ()
   local named_mats = {}
   local named_verts = {}

   local function add_mat (name, r, g, b)
      if not mat_dict:contains (name) then
	 named_mats[name] = material.lambert (color.rgb (r,g,b))
	 print ("named mat", name, color.rb (r,g,b), named_mats[name])
      end
   end

   local function add_vert (name, x, y, z)
      named_verts[name] = mesh:add_vertex (x, y, z)
   end

   local function map_vert (name)
      local vert = named_verts[name]
      if not vert then
	 lu.parse_err ("Unknown vertex name \"" .. name .. "\"")
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
	    lu.parse_err ("Unknown material name \"" .. name .. "\"")
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

   local C_CMD
      = P"c" * ((WS_NAME * WS_FLOAT * WS_FLOAT * WS_FLOAT) / add_mat)
   local V_CMD
      = P"v" * ((WS_NAME * WS_FLOAT * WS_FLOAT * WS_FLOAT) / add_vert)
   local F_CMD
      = P"f" * HWS * ((P"(" * WS_NAME^3 * HWS * P")" * WS_NAME^0) / add_face)
   local W_CMD  -- ignored
      = P"w" * (1 - P";")^0
   local CMD = (V_CMD + F_CMD + C_CMD) * P";" * HWS
   local LINE = CMD + HWS

   lu.parse_file (filename, LINE * lu.NL)
end
