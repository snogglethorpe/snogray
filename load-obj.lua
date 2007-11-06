-- load a .obj format mesh

local comment_char = string.byte ('#')
local v_char = string.byte ('v')
local f_char = string.byte ('f')
local spc_char = string.byte (' ')
local tab_char = string.byte ('\t')

function load_obj (filename, mesh, mat_map, xform)
   local s = io.open (filename, "r")

   local mat = mat_map:get_default ()

   for l in s:lines () do
      local cmd = string.byte (l)
      if not cmd or cmd == comment_char or cmd == spc_char or cmd == tab_char
      then
	 -- nothing

      elseif cmd == v_char then
	 local x,y,z
	    = string.match(l, "v%s+([-.e%d]+)%s+([-.e%d]+)%s+([-.e%d]+)")

	 mesh:add_vertex (pos (x, y, z) * xform)

      elseif cmd == f_char then

	 -- triangle
	 --
	 local v1, v2, v3 = string.match(l, "f%s+(%d+)%s+(%d+)%s+(%d+)%s*$")
	 if v1 then
	    mesh:add_triangle (v1 - 1, v2 - 1, v3 - 1, mat)
	 else

	    -- quad
	    --
	    local v1, v2, v3, v4
	       = string.match(l, "f%s+(%d+)%s+(%d+)%s+(%d+)%s+(%d+)%s*$")
	    if v1 then
	       mesh:add_triangle (v1 - 1, v2 - 1, v3 - 1, mat)
	       mesh:add_triangle (v3 - 1, v4 - 1, v1 - 1, mat)
	    else

	       -- general polygon
	       --
	       local first = nil
	       local prev = nil
	       for v in string.gmatch (l, "%s+(%d+)") do
		  v = v - 1
		  if not first then
		     first = v
		  else
		     if prev then
			mesh:add_triangle (first, prev, v, mat)
		     end
		     prev = v
		  end
	       end
	    end
	 end
      else
	 error ("invalid line in obj file \""..l.."\"")
      end
   end

   return true
end
