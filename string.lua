-- string.lua -- Miscellaneous string functions
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


local std_string = string -- standard Lua string module


-- module
--
local string = {}


-- Inherit from the standard Lua string module, so clients can just
-- call this module 'string'
--
setmetatable (string, {__index = std_string})


-- Return a string version of NUM, with commas added every 3rd place
--
function string.commify (num)
   num = tostring (num)
   return string.gsub (string.reverse (string.gsub (string.reverse (num),
						    "...", "%0,")),
		       "^,", "")
end

-- Return a string version of NUM, with commas added every 3rd place,
-- and either the phrase UNIT_NAME or UNITS_NAME appended, depending
-- on whether NUM has the value 1 or not.
--
function string.commify_with_units (num, unit_name, units_name)
   if num == 1 then
      return "1"..unit_name
   else
      return string.commify (num)..units_name
   end
end

-- Return a string version of NUM with its fractional part rounded to
-- PLACES decimal places (default 1), and commas added every 3rd place
-- in its integer part.
--
function string.round_and_commify (num, places)
   local frac_scale = 10^(places or 1)
   local scnum = math.floor (num * frac_scale + 0.5)
   local ip = math.floor (scnum / frac_scale)
   local fp = scnum - ip * frac_scale
   ip = string.commify (ip)
   if fp ~= 0 then
      ip = ip.."."..tostring(fp)
   end
   return ip
end


-- Concatenate a series of strings and separators, omitting the
-- proceeding separator for any string which is nil.
--
function string.sep_concat (str, ...)
   local i = 1
   while i < select ('#', ...) do
      local new = select (i + 1, ...)
      if not str then
	 str = new
      elseif new then
	 str = str..select (i, ...)..new
      end
      i = i + 2
   end
   return str
end


-- Return a string version of VAL padded on the left with spaces to
-- be at least LEN chars.
--
function string.left_pad (val, len)
   val = tostring (val)
   if #val < len then
      return string.rep (" ", len - #val)..val
   else
      return val
   end
end


-- Return a string version of VAL padded on the right with spaces to
-- be at least LEN chars.
--
function string.right_pad (val, len)
   val = tostring (val)
   if #val < len then
      return val..string.rep (" ", len - #val)
   else
      return val
   end
end


-- return the module
--
return string
