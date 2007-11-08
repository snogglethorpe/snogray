-- load a .obj format mesh

local lpeg = require 'lpeg'

-- local abbreviations for lpegp primitives
local P, R, S, C = lpeg.P, lpeg.R, lpeg.S, lpeg.C

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
local p_opt_horiz_ws = S" \t"^0	-- optional non-newline whitespace
local p_req_horiz_ws = S" \t"^1 -- required non-newline whitespace
local p_nl = S"\r\n\f"		-- newline char

-- line
local p_line = (1 - p_nl)^0
local p_line_nl = p_line * p_nl
local p_line_contents = C (p_line)

-- whitespace followed by numbers (useful shortcuts)
local p_ws_float = p_opt_ws * p_float
local p_ws_int = p_opt_ws * p_int

-- obj-file comment
local p_comment = P"#" * p_line


-- Signals an error with a simple message quoting the problem line
--
function parse_err (text, pos)
   local line_num = 1
   local count_pos = 1
   while count_pos < pos do
      line_num = line_num + 1
      print ("count_pos",count_pos)
      count_pos = p_line_nl:match (text, count_pos)
   end
   error ("parse error at position "..tostring(line_num)..": "
	  ..p_line_contents:match (text, pos))
end

function load_obj (filename, mesh, mat_map)
   local stream, err = io.open (filename, "r")
   if not stream then
      error (err)
   end

   local mat = mat_map:get_default ()

   local function add_vert (x, y, z)
      mesh:add_vertex (x, y, z)
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
   local p_cmd = p_v_cmd + p_f_cmd + p_comment + p_opt_horiz_ws
   local p_cmd_line = p_cmd * p_nl

   local text = stream:read'*a'

   local len = #text
   local pos = 1
   while pos <= len do
      local next_pos = p_cmd_line:match (text, pos)
      if not next_pos then
	 parse_err (text, pos)
      end
      pos = next_pos
   end

   return true
end
