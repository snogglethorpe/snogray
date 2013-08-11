-- color.lua -- Color support
--
--  Copyright (C) 2012  Miles Bader <miles@gnu.org>
--
-- This source code is free software; you can redistribute it and/or
-- modify it under the terms of the GNU General Public License as
-- published by the Free Software Foundation; either version 3, or (at
-- your option) any later version.  See the file COPYING for more details.
--
-- Written by Miles Bader <miles@gnu.org>
--

-- module
--
local color = {}

-- imports
--
local swig = require 'snogray.swig'
local table = require 'snogray.table'
local raw = require "snogray.snograw"


-- Return true if OBJ is a real color object.
--
local function is_color (obj)
   return swig.type (obj) == 'Color'
end
color.is_color = is_color


-- Table keywords used by the color.std function.
--
local color_keys = table.set{
   'r', 'red', 'g', 'green', 'b', 'blue', 'grey', 'gray',
   'i', 'intens', 'intensity', 'bright', 'brightness'
}

-- Return true if OBJ is something the color.std function would recognize.
--
local function is_color_spec (obj)
   local ot = type (obj)
   if ot == 'number' or is_color (obj) or (ot == 'string' and is_color (color[obj])) then
      return true
   elseif ot ~= 'table' then
      return false
   end

   for k,v in pairs (obj) do
      local kt = type (k)
      local vt = type (v)
      local inh = false
      if kt == 'number' then
	 if k == 1 and is_color_spec (v) then
	    inh = true
	 elseif k > 1 and inh then
	    return false
	 elseif k > 3 or vt ~= 'number' then
	    return false
	 end
      elseif not color_keys[k] or vt ~= 'number' then
	 return false
      end
   end

   return true
end
color.is_color_spec = is_color_spec

-- Returns a color corresponding for VAL, which can be a color, a table
-- (with keys in color_keys, above), a single grey-level, or a color
-- name.  This is sort of ad-hoc, and users should probably use
-- something more specific, such as color.rgb or color.grey.
--
function color.std (val, ...)
   if is_color (val) then
      return val
   else
      local t = type (val)

      if t == "number" then
	 return raw.Color (val, ...)
      elseif t == "string" then
	 local col = color[val]
	 if is_color (col) then
	    return col
	 else
	    error ("invalid color name: "..val, 2)
	 end
      elseif t == "table" then
	 local r,g,b

	 if not next (val) then
	    return color.white -- default to white if _nothing_ specified
	 end

	 if type (val[1]) == "number" then
	    if #val == 1 then
	       r, g, b = val[1], val[1], val[1]
	    else
	       r, g, b = val[1], val[2], val[3]
	    end
	 elseif val[1] then
	    local inherit = color.std (val[1])
	    r, g, b = inherit:r(), inherit:g(), inherit:b()
	 end

	 local grey = val.grey or val.gray
	 r = val.red or val.r or grey or r or 0
	 g = val.green or val.g or grey or g or 0
	 b = val.blue or val.b or grey or b or 0

	 local intens =
	    val.intensity
	    or val.intens
	    or val.i
	    or val.brightness
	    or val.bright

	 if intens then
	    local max = math.max (r,g,b)
	    if max > 0 then
	       local scale = intens / max
	       r = r * scale
	       g = g * scale
	       b = b * scale
	    end
	 end

	 local scale = val.scale or val.s
	 if scale then
	    r = r * scale
	    g = g * scale
	    b = b * scale
	 end

	 return raw.Color (r, g, b)

      else
	 error ("invalid color specification: "..tostring(val), 2)
      end
   end
end


-- Return a color object with the given red, blue, and green channels.
--
color.rgb = raw.Color

-- Return a neutral gray with the given intensity.
--
color.grey = raw.Color
color.gray = color.grey

-- Common colors.
--
color.white = color.grey (1)
color.black = color.grey (0)
color.red = color.rgb (1, 0, 0)
color.green = color.rgb (0, 1, 0)
color.blue = color.rgb (0, 0, 1)
color.cyan = color.rgb (0, 1, 1)
color.magenta = color.rgb (1, 0, 1)
color.yellow = color.rgb (1, 1, 0)


-- return the module
--
return color
