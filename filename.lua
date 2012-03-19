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

----------------------------------------------------------------
--
-- Filename manipulation

local match, substr = string.match, string.sub

-- Return the directory portion of FILENAME, or nil if it has none
--
function filename.directory (filename)
   return match (filename, "^(.*)/[^/]*$")
end

-- Return the extension (last part following a period) of FILENAME, or
-- nil if it has none.
--
function filename.extension (filename)
   return match (filename, "[.]([^./]*)$")
end

-- If FILENAME is relative, return it appended to DIR, otherwise just
-- return FILENAME.  If DIR is nil, then just return FILENAME.
--
function filename.in_directory (filename, dir)
   if dir and not match (filename, "^/") then
      filename = dir.."/"..filename
   end
   return filename
end

-- If FILENAME has DIR as a prefix, followed by a directory separator,
-- then return the portion of FILENAME following the directory
-- separator; otherwise, just return FILENAME.
-- 
function filename.relative (filename, dir)
   if substr (filename, 1, #dir + 1) == dir.."/" then
      return substr (filename, #dir + 2)
   else
      return filename
   end
end

-- return the module
--
return filename
