-- load a .obj format mesh

local lpeg = require 'lpeg'

-- local abbreviations for lpegp primitives
local P, R, S = lpeg.P, lpeg.R, lpeg.S

-- number components
local p_pm = S"+-"^-1
local p_digs = R"09"^1
local p_opt_digs = R"09"^0
local p_fmant = p_pm * ((P"." * p_digs) + (p_digs * (P"." * p_opt_digs)^-1))
local p_fexp = P"e" * p_pm * p_digs

-- whole numbers
local p_int = (p_pm * p_digs) / tonumber
local p_float = (p_fmant * p_fexp^-1) / tonumber

-- whitespace
local p_opt_ws = S" \t\r\n\f"^0	-- optional whitespace
local p_req_ws = S" \t\r\n\f"^1 -- required whitespace
local p_nl = S"\r\n\f"		-- newline char

-- whitespace followed by numbers (useful shortcuts)
local p_ws_float = p_opt_ws * p_float
local p_ws_int = p_opt_ws * p_int

-- obj-file comment
local p_comment = P"#" * (1 - p_nl)^0

function load_obj (filename, mesh, mat_map, xform)
   local s = io.open (filename, "r")

   local mat = mat_map:get_default ()

--    -- It's relatively expensive to create position objects and do matrix
--    -- transforms (this is largely overhead of the swig-generated
--    -- interface I think), so avoid doing so if we can.
--    --
--    if xform and xform:is_identity () then
--       xform = nil
--    end

   local function add_vert (x, y, z)
      mesh:add_vertex (x, y, z, xform)
   end

   local function add_poly (v1, v2, v3, ...)
      -- Convert vertex indices to zero-based index
      --
      v1 = v1 - 1
      v2 = v2 - 1
      v3 = v3 - 1

      -- We always add at least one triangle
      --
      mesh:add_triangle (v1, v2, v3, mat)

      -- Add extra triangles for more vertices
      --
      local prev = v3
      for i = 1, select ('#', ...) do
	 local v = select (i, ...) - 1
	 mesh:add_triangle (v1, prev, v, mat)
	 prev = v
      end
   end

   local p_v_cmd = P"v" * ((p_ws_float * p_ws_float * p_ws_float) / add_vert)
   local p_f_cmd = P"f" * (p_ws_int^2 / add_poly)
   local p_cmd = p_v_cmd + p_f_cmd + p_comment + p_opt_ws

   for line in s:lines () do
      if not p_cmd:match (line) then
	 error ("invalid line in obj file \""..line.."\"")
      end
   end

   return true
end
