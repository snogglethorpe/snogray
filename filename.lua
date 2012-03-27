-- filename.lua -- Filename manipulation functions
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
local filename = {}

local match, substr = string.match, string.sub


-- Return true if NAME is an absolute filename.
--
function filename.is_absolute (name)
   return match (name, "^/")
end

-- Return true if NAME is a relative filename.
--
function filename.is_relative (name)
   return not match (name, "^/")
end

-- Return the directory portion of NAME, or nil if it has none
--
function filename.directory (name)
   return match (name, "^(.*)/[^/]*$")
end

-- Return the extension (last part following a period) of NAME, or
-- nil if it has none.
--
function filename.extension (name)
   return match (name, "[.]([^./]*)$")
end

-- If NAME is relative, return it appended to DIR, otherwise just
-- return NAME.  If DIR is nil, then just return NAME.
--
function filename.in_directory (name, dir)
   if dir and filename.is_relative (name) then
      name = dir.."/"..name
   end
   return name
end

-- If NAME has DIR as a prefix, followed by a directory separator,
-- then return the portion of NAME following the directory separator;
-- otherwise, just return NAME.
-- 
function filename.relative (name, dir)
   if substr (name, 1, #dir + 1) == dir.."/" then
      return substr (name, #dir + 2)
   else
      return name
   end
end


-- return the module
--
return filename
