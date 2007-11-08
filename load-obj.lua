-- load a .obj format mesh

local lp = require 'lpeg'
local lu = require 'lpeg-utils'

-- obj-file comment
local p_comment = lp.P"#" * lu.p_line

function load_obj (filename, mesh, mat_map)
   local mat = mat_map:get_default ()

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

   local p_v_cmd
      = lp.P"v" * ((lu.p_ws_float * lu.p_ws_float * lu.p_ws_float) / add_vert)
   local p_f_cmd
      = lp.P"f" * (lu.p_ws_int^2 / add_poly)
   local p_cmd
      = p_v_cmd + p_f_cmd + p_comment + lu.p_opt_horiz_ws

   lu.parse_file (filename, p_cmd * lu.p_nl)

   return true
end
